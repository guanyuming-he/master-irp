#! /bin/bash
# The file is licensed under the GNU GPL v3.
# Copyright (C) Guanyuming He 2025
#
# The script is written for cron to run automatically. It will setup the
# environment first for cron and then run updater.
#
# Arg1 : path to project root dir.
# Arg2 : path to user home dir, where conda is.
#

if [ -z "$1" ]; then
	echo "arg1, project root, is missing." >&2
	exit 1
fi
if [ -z "$2" ]; then
	echo "arg2, user home, is missing." >&2
	exit 1
fi

# Activate conda
if [ -f "$2anaconda3/bin/activate" ]; then
	source "$2anaconda3/bin/activate" irp
else
	echo "Anaconda not found at $2anaconda/bin/activate."
	exit 1
fi

# Activate my irp env.
if [ "$CONDA_DEFAULT_ENV" != "irp" ]; then
	conda activate irp
fi

# Now, run the pipeline.
cd $1
mkdir -p ./logs
./bin/updater ./db 600 2>./logs/updater_log
