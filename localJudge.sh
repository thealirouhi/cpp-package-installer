#!/usr/bin/env bash


# Config
BINARY="./installer"
IN_DIR="./in"
OUT_DIR="./out"
TIME_LIMIT=3
NUM_TESTS=100

# Colours
R='\033[0;31m'
G='\033[0;32m'
Y='\033[0;33m'
B='\033[0;34m'
C='\033[0;36m'
W='\033[1;37m'
D='\033[2;37m'
N='\033[0m'
BOLD='\033[1m'

hr()  { printf "${D}------------------------------------------------------------${N}\n"; }
hr2() { printf "${B}============================================================${N}\n"; }

# ── Platform detection ───────────────────────────────────────
IS_WINDOWS=0
case "$(uname -s 2>/dev/null)" in
    MINGW*|MSYS*|CYGWIN*) IS_WINDOWS=1 ;;
esac
# Also catch Git Bash running inside a Windows environment
[[ "${OS}" == "Windows_NT" ]] && IS_WINDOWS=1

# ── Resolve make — prefer Windows-native make on Windows ─────
MAKE_CMD=""
if [ $IS_WINDOWS -eq 1 ]; then
    # On Windows, prefer MinGW make candidates first to avoid WSL's make
    for candidate in mingw32-make.exe mingw64-make.exe mingw32-make mingw64-make make.exe make; do
        if command -v "$candidate" &>/dev/null; then
            # Reject any candidate that lives under /usr (that's WSL's make)
            CANDIDATE_PATH="$(command -v "$candidate" 2>/dev/null)"
            if [[ "$CANDIDATE_PATH" != /usr/* ]]; then
                MAKE_CMD="$candidate"
                break
            fi
        fi
    done
    # If nothing found outside /usr, fall back to whatever exists
    if [ -z "$MAKE_CMD" ]; then
        for candidate in mingw32-make.exe mingw64-make.exe mingw32-make mingw64-make make.exe make; do
            if command -v "$candidate" &>/dev/null; then
                MAKE_CMD="$candidate"
                break
            fi
        done
    fi
else
    for candidate in make; do
        if command -v "$candidate" &>/dev/null; then
            MAKE_CMD="$candidate"
            break
        fi
    done
fi

# ── Resolve gcc — detect if it's WSL/cross-compiler ─────────
GCC_CMD=""
GCC_IS_NATIVE=1   # assume native unless proven otherwise
if command -v gcc &>/dev/null; then
    GCC_CMD="gcc"
    GCC_PATH="$(command -v gcc)"
    # If gcc lives under /usr on a Windows host → it's WSL gcc → produces ELF
    if [ $IS_WINDOWS -eq 1 ] && [[ "$GCC_PATH" == /usr/* ]]; then
        GCC_IS_NATIVE=0
    fi
fi

# ── Resolve timeout ──────────────────────────────────────────
TIMEOUT_CMD=""
for candidate in timeout gtimeout timeout.exe; do
    if command -v "$candidate" &>/dev/null; then
        if "$candidate" --version 2>&1 | grep -qi 'gnu\|coreutils'; then
            TIMEOUT_CMD="$candidate"; break
        elif [ "$candidate" = "timeout" ] && "$candidate" 1 true 2>/dev/null; then
            TIMEOUT_CMD="$candidate"; break
        fi
    fi
done

# ── mktemp portability ───────────────────────────────────────
make_tmpdir() {
    if command -v mktemp &>/dev/null; then
        mktemp -d /tmp/judge_XXXXXX 2>/dev/null || mktemp -d
    else
        local d="/tmp/judge_$$"
        mkdir -p "$d"
        echo "$d"
    fi
}

# ── Elapsed time (pure-bash $SECONDS builtin) ────────────────
elapsed_start() { _T_START=$SECONDS; }
elapsed_end()   { echo "$(( SECONDS - _T_START ))s"; }

# ── Normalize: strip \r, remove leading blank lines ──────────
normalize() {
    sed 's/\r//' "$1" | awk 'NF{found=1} found{print}' | sed -e '$a\'
}

# ── Check if a binary is ELF (Linux) format ──────────────────
# Returns 0 (true) if it IS an ELF binary
is_elf_binary() {
    local f="$1"
    [ ! -f "$f" ] && return 1
    # Read the first 4 bytes — ELF magic is: 0x7f 45 4c 46 (i.e. .ELF)
    local magic
    magic=$(head -c 4 "$f" 2>/dev/null | od -An -tx1 2>/dev/null | tr -d ' \n')
    [[ "$magic" == "7f454c46" ]]
}

# ════════════════════════════════════════════════════════════
clear
hr2
printf "${W}  ⚖  LOCAL JUDGE${N}\n"
hr2
echo ""

# Platform info
if [ $IS_WINDOWS -eq 1 ]; then
    printf "  ${C}Platform :${N}  Windows (Git Bash / MinGW)\n"
else
    printf "  ${C}Platform :${N}  $(uname -s)\n"
fi
printf "  ${C}Make     :${N}  ${MAKE_CMD:-not found}\n"
printf "  ${C}GCC      :${N}  ${GCC_CMD:-not found}$([ $GCC_IS_NATIVE -eq 0 ] && printf " ${R}[WSL/cross — produces Linux ELF!]${N}")\n"
printf "  ${C}Timeout  :${N}  ${TIMEOUT_CMD:-disabled}\n"

if [ -z "$TIMEOUT_CMD" ]; then
    printf "  ${Y}  ⚠  TLE detection disabled.${N}\n"
    [ $IS_WINDOWS -eq 1 ] && printf "  ${D}     (install coreutils via Git Bash setup to enable)${N}\n"
fi
echo ""

# ── Sanity checks ────────────────────────────────────────────
FATAL=0
if [ -z "$MAKE_CMD" ]; then
    printf "  ${R}✖  No make found.${N}\n"
    printf "  ${D}     Windows: place mingw32-make.exe next to this script or add MinGW/bin to PATH.${N}\n"
    FATAL=1
fi
# Warn (not fatal) if gcc is WSL's — the Makefile might still work if it invokes gcc.exe by full path
if [ $IS_WINDOWS -eq 1 ] && [ $GCC_IS_NATIVE -eq 0 ]; then
    printf "  ${Y}  ⚠  gcc on PATH is WSL's gcc (${GCC_PATH}).${N}\n"
    printf "  ${D}     It will produce a Linux ELF binary that cannot run on Windows.${N}\n"
    printf "  ${D}     Fix: add MinGW's bin folder to PATH so its gcc.exe is found first.${N}\n\n"
fi
if [ ! -d "$IN_DIR" ];  then printf "  ${R}✖  Input folder '${IN_DIR}' not found${N}\n";  FATAL=1; fi
if [ ! -d "$OUT_DIR" ]; then printf "  ${R}✖  Output folder '${OUT_DIR}' not found${N}\n"; FATAL=1; fi
[ $FATAL -eq 1 ] && { printf "\n  ${R}${BOLD}Fix the above and re-run.${N}\n\n"; exit 1; }

# ── Resolve binary (handle .exe suffix on Windows) ───────────
BINARY_RESOLVED="$BINARY"
if [ $IS_WINDOWS -eq 1 ] && [ ! -f "$BINARY" ] && [ -f "${BINARY}.exe" ]; then
    BINARY_RESOLVED="${BINARY}.exe"
fi

# ── Build ─────────────────────────────────────────────────────
printf "${W}[BUILD]${N} Running ${C}${MAKE_CMD}${N}...\n"
hr
"$MAKE_CMD" --no-print-directory 2>&1 | sed 's/^/  /'
BUILD_STATUS=${PIPESTATUS[0]}
[ -z "$BUILD_STATUS" ] && BUILD_STATUS=$?
hr

[ $BUILD_STATUS -ne 0 ] && { printf "\n  ${R}${BOLD}✖  Compilation FAILED - aborting.${N}\n\n"; exit 1; }

# Re-resolve after build
if [ ! -f "$BINARY_RESOLVED" ]; then
    if   [ -f "${BINARY}.exe" ]; then BINARY_RESOLVED="${BINARY}.exe"
    elif [ -f "$BINARY" ];       then BINARY_RESOLVED="$BINARY"
    else printf "\n  ${R}${BOLD}✖  Binary '${BINARY}' not found after make.${N}\n\n"; exit 1
    fi
fi

chmod +x "$BINARY_RESOLVED" 2>/dev/null

# ── ELF-on-Windows guard ─────────────────────────────────────
if [ $IS_WINDOWS -eq 1 ] && is_elf_binary "$BINARY_RESOLVED"; then
    printf "\n  ${R}${BOLD}✖  WRONG BINARY FORMAT — ELF detected on Windows!${N}\n\n"
    printf "  ${Y}  The compiled binary is a Linux ELF file, not a Windows .exe.${N}\n"
    printf "  ${Y}  This happens when WSL's gcc compiles the code instead of MinGW's gcc.${N}\n\n"
    printf "  ${W}  How to fix:${N}\n"
    printf "  ${D}  1. Find where MinGW is installed (e.g. C:\\mingw64\\bin)${N}\n"
    printf "  ${D}  2. In Git Bash run:${N}\n"
    printf "  ${C}       export PATH=\"/c/mingw64/bin:\$PATH\"${N}\n"
    printf "  ${D}  3. To make it permanent, add that line to ~/.bashrc${N}\n"
    printf "  ${D}  4. Re-run this judge script.${N}\n\n"
    exit 1
fi

printf "  ${G}${BOLD}✔  Build successful  ${N}${D}(${BINARY_RESOLVED})${N}\n\n"

# ── Counters ──────────────────────────────────────────────────
cnt_ac=0; cnt_wa=0; cnt_tle=0; cnt_re=0; cnt_mf=0; total=0
declare -a results times details

# ── Run tests ─────────────────────────────────────────────────
printf "${W}[JUDGE]${N} Running ${C}${NUM_TESTS}${N} tests  (TL = ${TIME_LIMIT}s)\n"
hr

TMPDIR_JUDGE=$(make_tmpdir)

for i in $(seq 1 $NUM_TESTS); do

    INPUT_FILE="${IN_DIR}/in${i}.txt"
    EXPECTED_FILE="${OUT_DIR}/out${i}.txt"
    ACTUAL_FILE="${TMPDIR_JUDGE}/actual_${i}.txt"
    NORM_ACTUAL="${TMPDIR_JUDGE}/norm_a_${i}.txt"
    NORM_EXPECTED="${TMPDIR_JUDGE}/norm_e_${i}.txt"
    ERR_FILE="${TMPDIR_JUDGE}/err_${i}.txt"

    if [ ! -f "$INPUT_FILE" ] && [ ! -f "$EXPECTED_FILE" ]; then
        results[$i]="MISSING"; times[$i]="-"; details[$i]="in${i}.txt and out${i}.txt not found"
        cnt_mf=$((cnt_mf+1)); total=$((total+1)); continue
    fi
    if [ ! -f "$INPUT_FILE" ]; then
        results[$i]="MISSING"; times[$i]="-"; details[$i]="in${i}.txt not found"
        cnt_mf=$((cnt_mf+1)); total=$((total+1)); continue
    fi
    if [ ! -f "$EXPECTED_FILE" ]; then
        results[$i]="MISSING"; times[$i]="-"; details[$i]="out${i}.txt not found"
        cnt_mf=$((cnt_mf+1)); total=$((total+1)); continue
    fi

    total=$((total+1))
    elapsed_start

    if [ -n "$TIMEOUT_CMD" ]; then
        "$TIMEOUT_CMD" "$TIME_LIMIT" "$BINARY_RESOLVED" < "$INPUT_FILE" > "$ACTUAL_FILE" 2>"$ERR_FILE"
        EXIT_CODE=$?
    else
        "$BINARY_RESOLVED" < "$INPUT_FILE" > "$ACTUAL_FILE" 2>"$ERR_FILE"
        EXIT_CODE=$?
    fi

    times[$i]=$(elapsed_end)

    if [ -n "$TIMEOUT_CMD" ] && [ $EXIT_CODE -eq 124 ]; then
        results[$i]="TLE"; details[$i]="exceeded ${TIME_LIMIT}s"
        cnt_tle=$((cnt_tle+1))
    elif [ $EXIT_CODE -ne 0 ]; then
        results[$i]="RE"
        STDERR_MSG=$(head -1 "$ERR_FILE" 2>/dev/null)
        # Friendlier message for the ELF-on-Windows case that slips through
        if [ $EXIT_CODE -eq 126 ] && [ $IS_WINDOWS -eq 1 ]; then
            details[$i]="exit 126 — binary is likely a Linux ELF, not a Windows .exe (wrong gcc)"
        else
            details[$i]="exit code ${EXIT_CODE}${STDERR_MSG:+ | ${STDERR_MSG}}"
        fi
        cnt_re=$((cnt_re+1))
    else
        normalize "$ACTUAL_FILE"   > "$NORM_ACTUAL"
        normalize "$EXPECTED_FILE" > "$NORM_EXPECTED"
        if diff -q "$NORM_ACTUAL" "$NORM_EXPECTED" &>/dev/null; then
            results[$i]="AC"; details[$i]=""; cnt_ac=$((cnt_ac+1))
        else
            results[$i]="WA"
            details[$i]=$(diff "$NORM_ACTUAL" "$NORM_EXPECTED" | grep '^[<>]' | head -1)
            cnt_wa=$((cnt_wa+1))
        fi
    fi
done

# ── Results table ─────────────────────────────────────────────
echo ""
printf "${W}[RESULTS]${N}\n"
hr
printf "  ${D}%-8s  %-26s  %-6s  %s${N}\n" "Test" "Verdict" "Time" "Info"
printf "  ${D}%-8s  %-26s  %-6s  %s${N}\n" "--------" "--------------------------" "------" "----"

for i in $(seq 1 $NUM_TESTS); do
    [ -z "${results[$i]}" ] && continue
    case "${results[$i]}" in
        AC)      BADGE="${G}${BOLD}  ✔  Accepted            ${N}" ;;
        WA)      BADGE="${R}${BOLD}  ✘  Wrong Answer         ${N}" ;;
        TLE)     BADGE="${Y}${BOLD}  ⏱  Time Limit Exceeded  ${N}" ;;
        RE)      BADGE="${R}${BOLD}  ⚡  Runtime Error         ${N}" ;;
        MISSING) BADGE="${D}  ?  Missing Files         ${N}" ;;
    esac
    printf "  ${W}%-8s${N}  %b  ${D}%-6s${N}  ${D}%s${N}\n" \
        "#${i}" "$BADGE" "${times[$i]}" "${details[$i]}"
done

# ── Summary ───────────────────────────────────────────────────
echo ""
hr2
printf "${W}  SUMMARY${N}\n"
hr2
echo ""

PASS_RATE=0
[ $total -gt 0 ] && PASS_RATE=$(awk "BEGIN{printf \"%.1f\", ($cnt_ac/$total)*100}")

printf "  ${G}${BOLD}%-32s${N}  %d\n" "✔  Accepted"            $cnt_ac
printf "  ${R}${BOLD}%-32s${N}  %d\n" "✘  Wrong Answer"         $cnt_wa
printf "  ${Y}${BOLD}%-32s${N}  %d\n" "⏱  Time Limit Exceeded"  $cnt_tle
printf "  ${R}${BOLD}%-32s${N}  %d\n" "⚡  Runtime Error"         $cnt_re
[ $cnt_mf -gt 0 ] && \
printf "  ${D}${BOLD}%-32s${N}  %d\n" "?  Missing Files"         $cnt_mf

echo ""
hr
printf "\n  Score  ${W}${BOLD}%d / %d${N}  (${C}%s%%${N})\n\n" $cnt_ac $total "$PASS_RATE"

BAR_WIDTH=50
FILLED=0
[ $total -gt 0 ] && FILLED=$(awk "BEGIN{printf \"%d\", ($cnt_ac/$total)*$BAR_WIDTH}")
EMPTY=$(( BAR_WIDTH - FILLED ))

printf "  ${G}"
for ((k=0; k<FILLED; k++)); do printf "#"; done
printf "${D}"
for ((k=0; k<EMPTY; k++));  do printf "."; done
printf "${N}\n\n"

hr2

rm -rf "$TMPDIR_JUDGE"

FAILED=$(( cnt_wa + cnt_tle + cnt_re + cnt_mf ))
exit $FAILED