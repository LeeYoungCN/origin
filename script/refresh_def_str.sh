#!/usr/bin/env bash
echo "Bash version: $BASH_VERSION"
if [ -z "$BASHRC_LOADED" ]; then
    # shellcheck disable=SC1091
    source "${HOME}/.bashrc"
fi

SCRIPT_DIR="$(
    cd "$(dirname "$0")" || exit 1
    pwd
)"
ROOT_DIR="$(
    cd "${SCRIPT_DIR}/.." || exit 1
    pwd
)"

readonly SCRIPT_DIR
readonly ROOT_DIR

# shellcheck disable=SC1091
source "${SCRIPT_DIR}/common_func.sh"

cd "${ROOT_DIR}" || exit 1


LIBS_DIR="${ROOT_DIR}/src/libs"

find "${LIBS_DIR}" -mindepth 1 -maxdepth 1 -type d -print0 |
        while IFS= read -r -d '' lib_dir; do
            lib_name=$(basename ${lib_dir})
            inc_dir=${lib_dir}/include/
            find "${inc_dir}" -type f -name "*.h"  -print0 |
                while IFS= read -r -d '' file; do
                    echo ${file}
                    first_ifndef=$(grep -m 1 '#ifndef' "${file}")
                    first_define=$(grep -m 1 '#define' "${file}")
                    last_endif=$(tac "${file}" | grep -m 1 "#endif" | tac)
                    new_def_str="${file//${inc_dir}/}"
                    new_def_str=$(echo "$new_def_str" | sed 's/[/.]/_/g' | tr '[:lower:]' '[:upper:]')
                    echo ${new_def_str}
                    new_ifndef="#ifndef ${new_def_str}"
                    new_define="#define ${new_def_str}"
                    new_endif="#endif  // ${new_def_str}"
                    replace_text "${first_ifndef}" "${new_ifndef}" "${file}"
                    replace_text "${first_define}" "${new_define}" "${file}"
                    replace_text "${last_endif}"   "${new_endif}"  "${file}"
                done
        done


