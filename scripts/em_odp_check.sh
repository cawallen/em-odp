#!/bin/bash
#
# This script is a clean up for the EM-ODP project src files.
#
# Usage
# ./scripts/em_odp_check <path/filename>
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# checkpatch ignores:
IGNORE='NEW_TYPEDEFS,'
IGNORE+='COMPARISON_TO_NULL,'
IGNORE+='PREFER_KERNEL_TYPES,'
IGNORE+='MACRO_WITH_FLOW_CONTROL,'
IGNORE+='LONG_LINE_STRING,'
IGNORE+='PREFER_PRINTF,'
IGNORE+='CONST_STRUCT,'
IGNORE+='FUNCTION_ARGUMENTS,'
IGNORE+='MACRO_ARG_REUSE,'
IGNORE+='SPDX_LICENSE_TAG,'
IGNORE+='C99_COMMENT_TOLERANCE,' # C99-style comments '//' reported as error
IGNORE+='PREFER_FALLTHROUGH,'
IGNORE+='SSCANF_TO_KSTRTO,'
IGNORE+='PREFER_ALIGNED',
IGNORE+='MACRO_ARG_PRECEDENCE', # ignore sizeof_field(type, field) CHECK-warning
IGNORE+='PREFER_DEFINED_ATTRIBUTE_MACRO',
IGNORE+='STRNCPY',
IGNORE+='TRACING_LOGGING'

# Run cleanfile
"$DIR"/cleanfile "$1" 2> /dev/null

# Run checkpatch
"$DIR"/checkpatch.pl --file --no-tree --ignore $IGNORE --mailback --strict --terse \
 --no-summary --show-types "$1" 2> /dev/null
ret_checkpatch=$?

# Run codespell (if installed) with default dictionary-file from installation
ret_codespell=0
if which codespell > /dev/null; then
	codespell "$1" --ignore-words-list=ptd,numer,stdio,endcode
	ret_codespell=$?
fi

# Exit with 0 only if both commands were successful
if [[ $ret_checkpatch -eq 0 && $ret_codespell -eq 0 ]]; then
  exit 0
else
  exit 1
fi
