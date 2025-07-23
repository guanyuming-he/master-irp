#! /bin/bash
# The file is licensed under the GNU GPL v3.
# Copyright (C) Guanyuming He 2025
#
# The script is written for cron to run automatically. It will setup the
# environment first for cron and then run the pipeline.
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

# Start ollama
ollama serve >/tmp/ollama.log 2>&1 &
OLLAMA_PID=$!
# wait for a few seconds for it to be ready
sleep 5

# Activate conda
if [ -f "$2anaconda3/bin/activate" ]; then
	source "$2anaconda3/bin/activate" irp
else
	echo "Anaconda not found at $2anaconda3/bin/activate."
	kill $OLLAMA_PID
	exit 1
fi

# Activate my irp env.
if [ "$CONDA_DEFAULT_ENV" != "irp" ]; then
	conda activate irp
fi

# Now run the pipeline.
cd $1
mkdir -p ./logs
python3 ./src/pipeline/llm_pipeline.py 2>./logs/pipeline_log

# 3. Stop the `ollama serve` process
kill "$OLLAMA_PID"
wait "$OLLAMA_PID" 2>/dev/null

