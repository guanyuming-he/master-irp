"""
@author Guanyuming He
Copyright (C) Guanyuming He 2025
The file is licensed under the GNU GPL v3.

Scheduler

This files defines a script that installs/uninstalls
schedueling tasks on various operating systems.
"""

import json
import platform
import subprocess
from pathlib import Path
import os
import plistlib
import sys

def cron_tag(label):
	return f"# JOB:{label}"

def install_cron_job(label, cfg):
	time = cfg['time']
	hour, minute = map(int, time.split(":"))
	command = cfg['command']
	schedule = cfg['schedule']

	if schedule == "weekly":
		day = ((cfg['day'] % 7) or 7) - 1
		cron_expr = f"{minute} {hour} * * {day} {command}"

	elif schedule == "monthly":
		day = cfg['day']
		cron_expr = f"{minute} {hour} {day} * * {command}"

	elif schedule == "every_x_days":
		x = cfg['every_x_days']
		wrapper_path = Path.home() / f".local/bin/{label}_wrapper.sh"
		wrapper_path.parent.mkdir(parents=True, exist_ok=True)

		with open(wrapper_path, "w") as f:
			f.write(f"""#!/bin/bash
X={x}
STATEFILE="{Path.home()}/.{label}_last_run"
NOW=$(date +%s)
LAST=0
if [ -f "$STATEFILE" ]; then
	LAST=$(cat "$STATEFILE")
fi
DIFF=$(( (NOW - LAST) / 86400 ))
if [ "$DIFF" -ge "$X" ]; then
	{command}
	date +%s > "$STATEFILE"
fi
""")
		os.chmod(wrapper_path, 0o755)
		cron_expr = f"{minute} {hour} * * * {wrapper_path}"
	else:
		raise ValueError("Invalid schedule type")

	tag = cron_tag(label)
	job_line = f"{tag}\n{cron_expr}"

	try:
		existing = subprocess.check_output(
			["crontab", "-l"],
			stderr=subprocess.DEVNULL
		).decode()
	except subprocess.CalledProcessError:
		existing = ""

	if tag in existing:
		print(f"[cron] Job '{label}' already installed.")
		return

	updated = existing + "\n" + job_line + "\n"
	subprocess.run(["crontab", "-"], input=updated.encode())
	print(f"[cron] Job '{label}' installed.")

def uninstall_cron_job(label):
	tag = cron_tag(label)

	try:
		existing = subprocess.check_output(
			["crontab", "-l"],
			stderr=subprocess.DEVNULL
		).decode()
	except subprocess.CalledProcessError:
		print(f"[cron] No crontab found for job '{label}'.")
		return

	lines = existing.splitlines()
	filtered = []
	skip = False
	for line in lines:
		if line.strip() == tag:
			skip = True
			continue
		if skip:
			skip = False
			continue
		filtered.append(line)

	updated = "\n".join(filtered) + "\n"
	subprocess.run(["crontab", "-"], input=updated.encode())
	print(f"[cron] Job '{label}' uninstalled.")

def install_launchd_job(label, cfg):
	job_label = f"com.user.{label}"
	plist_path = (
		Path.home() /
		"Library/LaunchAgents" /
		f"{job_label}.plist"
	)

	time = cfg['time']
	hour, minute = map(int, time.split(":"))
	command = cfg['command']
	schedule = cfg['schedule']
	catch_up = cfg.get("catch_up", False)

	plist = {
		"Label": job_label,
		"ProgramArguments": ["/bin/bash", "-c", command],
		"StartCalendarInterval": {},
		"StandardOutPath": f"{Path.home()}/Library/Logs/{job_label}.out",
		"StandardErrorPath": f"{Path.home()}/Library/Logs/{job_label}.err",
		"RunAtLoad": not catch_up
	}

	if schedule == "weekly":
		weekday = ((cfg['day'] % 7) or 7) - 1
		plist["StartCalendarInterval"] = {
			"Hour": hour,
			"Minute": minute,
			"Weekday": weekday
		}

	elif schedule == "monthly":
		plist["StartCalendarInterval"] = {
			"Hour": hour,
			"Minute": minute,
			"Day": cfg['day']
		}

	elif schedule == "every_x_days":
		x = cfg['every_x_days']
		wrapper_path = Path.home() / f".local/bin/{label}_wrapper.sh"
		wrapper_path.parent.mkdir(parents=True, exist_ok=True)

		with open(wrapper_path, "w") as f:
			f.write(f"""#!/bin/bash
X={x}
STATEFILE="{Path.home()}/.{label}_last_run"
NOW=$(date +%s)
LAST=0
if [ -f "$STATEFILE" ]; then
	LAST=$(cat "$STATEFILE")
fi
DIFF=$(( (NOW - LAST) / 86400 ))
if [ "$DIFF" -ge "$X" ]; then
	{command}
	date +%s > "$STATEFILE"
fi
""")
		os.chmod(wrapper_path, 0o755)
		plist["ProgramArguments"] = [
			"/bin/bash", str(wrapper_path)
		]
		plist["StartCalendarInterval"] = {
			"Hour": hour,
			"Minute": minute
		}
	else:
		raise ValueError("Invalid schedule type")

	with open(plist_path, "wb") as f:
		plistlib.dump(plist, f)

	subprocess.run(["launchctl", "load", str(plist_path)])
	print(f"[launchd] Job '{label}' installed at {plist_path}")

def uninstall_launchd_job(label):
	job_label = f"com.user.{label}"
	plist_path = (
		Path.home() /
		"Library/LaunchAgents" /
		f"{job_label}.plist"
	)

	if not plist_path.exists():
		print(f"[launchd] Job '{label}' not found.")
		return

	subprocess.run(["launchctl", "unload", str(plist_path)])
	plist_path.unlink()
	print(f"[launchd] Job '{label}' removed.")

def main():
	if len(sys.argv) != 3:
		print("Usage: install_scheduler.py [install|uninstall] config.json")
		return

	mode = sys.argv[1]
	with open(sys.argv[2], "r") as f:
		config = json.load(f)

	schedules = config.get("schedules")
	if not schedules:
		print("No 'schedules' found in config.")
		return

	system = platform.system()

	for label, cfg in schedules.items():
		if system == "Linux":
			if mode == "install":
				install_cron_job(label, cfg)
			elif mode == "uninstall":
				uninstall_cron_job(label)
		elif system == "Darwin":
			if mode == "install":
				install_launchd_job(label, cfg)
			elif mode == "uninstall":
				uninstall_launchd_job(label)
		else:
			print(f"Unsupported OS: {system}")
			break

if __name__ == "__main__":
	main()

