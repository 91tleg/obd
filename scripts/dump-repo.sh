#!/usr/bin/env bash

EXCLUDES=".git build .vscode .venv build unity"

print_tree() {
    local dir="$1"
    local prefix="$2"

    local entries
    entries=$(ls -A "$dir" 2>/dev/null | grep -v -E "$(echo $EXCLUDES | sed 's/ /|/g')" | sort)

    local count=$(echo "$entries" | wc -l | tr -d ' ')
    local i=0

    echo "$entries" | while read -r entry; do
        [ -z "$entry" ] && continue
        i=$((i + 1))
        local connector="├──"
        [ "$i" -eq "$count" ] && connector="└──"

        echo "${prefix}${connector} $entry"

        if [ -d "$dir/$entry" ]; then
            local new_prefix="$prefix"
            [ "$i" -eq "$count" ] && new_prefix="${prefix}    " || new_prefix="${prefix}│   "
            print_tree "$dir/$entry" "$new_prefix"
        fi
    done
}

echo "$(basename "$(pwd)")"
print_tree "."
