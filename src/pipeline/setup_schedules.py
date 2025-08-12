"""
@author Guanyuming He
Copyright (C) Guanyuming He 2025
The file is licensed under the GNU GPL v3.

Use cron to schedule the tasks (may support Windows if I have time). One may
need specific permissions to execute this script successfully.  

The script is designed to be 
1. Correct today and in the unknown future:
	Each interaction with cron is isolated in specific methods.
	Also, when I need to change in the future, the platform-specific backend is
	abstracted into a class, and I only need to change that. This defensively
	makes the program more likely to be correct with changes.
2. Easy to understand:
	My interfaces with scheduler is simple, install, remove, list, and clear
	all. Each backend implements this simple interface.
3. Ready for change:
	Abstracting backend away makes each OS's work independent of each other.
"""

import os
import platform
import subprocess
import tempfile
from typing import List, Optional, Tuple
from enum import Enum

from config import Config, Schedule, ScheduleType


class PlatformType(Enum):
	"""
	Supported platforms for task scheduling.
	"""
	LINUX = "linux"
	DARWIN = "darwin"  # macOS
	UNKNOWN = "unknown" # not supported. May support Windows if I have time.


class SchedulerBackend():
	"""
	Abstract base class for platform-specific scheduling backends.
	
	Each backend implements the platform-specific logic for:
	- Installing scheduled tasks
	- Removing scheduled tasks
	- Listing existing tasks
	"""
	
	def install_schedule(self, schedule: Schedule) -> bool:
		"""
		Install a scheduled task on the target platform.
		
		@param schedule The Schedule object to install
		@return True if successful, False otherwise
		"""
		raise NotImplementedError("Abstract method")
	
	def remove_schedule(self, schedule_name: str) -> bool:
		"""
		Remove a scheduled task by name.
		
		@param schedule_name The name of the schedule to remove
		@return True if successful, False otherwise
		"""
		raise NotImplementedError("Abstract method")
	
	def list_schedules(self) -> List[str]:
		"""
		List all schedules managed by this backend.
		
		@return List of schedule names
		"""
		raise NotImplementedError("Abstract method")
	
	def clear_all_schedules(self) -> bool:
		"""
		Remove all schedules managed by this backend.
		
		@return True if successful, False otherwise
		"""
		raise NotImplementedError("Abstract method")


class CronBackend(SchedulerBackend):
	"""
	Cron-based scheduling backend for GNU/Linux and macOS.
	
	This backend manages cron entries by:
	1. Reading the current crontab
	2. Filtering out existing entries managed by this tool
	3. Adding new entries
	4. Installing the updated crontab
	
	To identify managed entries, we use a comment marker.
	"""
	
	MANAGED_MARKER = "# MANAGED BY Guanyuming's IRP Project scheduler"
	
	def __init__(self):
		self.cron_command = self._find_cron_command()
	
	def _find_cron_command(self) -> str:
		"""
		Find the appropriate cron command for the system.
		
		@return The cron command path
		"""
		# Most systems use 'crontab'
		for cmd in ['crontab', '/usr/bin/crontab', '/bin/crontab']:
			if os.path.exists(cmd.split()[0]) or \
			   subprocess.run(['which', cmd], 
			                 capture_output=True).returncode == 0:
				return cmd
		raise RuntimeError("crontab command not found")
	
	def _get_current_crontab(self) -> List[str]:
		"""
		Get the current crontab entries as a list of lines.
		
		@return List of crontab lines, empty if no crontab exists
		"""
		try:
			result = subprocess.run(
				[self.cron_command, '-l'],
				capture_output=True,
				text=True,
				check=True
			)
			return result.stdout.strip().split('\n') if result.stdout.strip() else []
		except subprocess.CalledProcessError:
			# No crontab exists yet
			return []
	
	def _set_crontab(self, lines: List[str]) -> bool:
		"""
		Set the crontab to the given lines.
		
		@param lines List of crontab lines
		@return True if successful, False otherwise
		"""
		try:
			with tempfile.NamedTemporaryFile(mode='w', delete=False) as f:
				f.write('\n'.join(lines))
				if lines:  # Add final newline if there are lines
					f.write('\n')
				temp_path = f.name
			
			result = subprocess.run(
				[self.cron_command, temp_path],
				capture_output=True,
				text=True
			)
			
			os.unlink(temp_path)
			return result.returncode == 0
		except Exception:
			return False
	
	def _schedule_to_cron_time(self, schedule: Schedule) -> str:
		"""
		Convert a Schedule object to cron time format.
		
		@param schedule The Schedule to convert
		@return Cron time string (minute hour day month weekday)
		"""
		minute = schedule.time.minute
		hour = schedule.time.hour
		
		if schedule.stype == ScheduleType.EVERY_X_DAYS:
			# Run every X days: use day-of-month field with step
			return f"{minute} {hour} */{schedule.day} * *"
		elif schedule.stype == ScheduleType.WEEKLY:
			# Run weekly on specific weekday (1=Monday, 7=Sunday in our system)
			# Convert to cron format (0=Sunday, 6=Saturday)
			cron_weekday = schedule.day % 7
			return f"{minute} {hour} * * {cron_weekday}"
		elif schedule.stype == ScheduleType.MONTHLY:
			# Run monthly on specific day of month
			return f"{minute} {hour} {schedule.day} * *"
		else:
			raise ValueError(f"Unknown schedule type: {schedule.stype}")
	
	def _get_managed_lines(self) -> List[str]:
		"""
		Get all crontab lines that are managed by this tool.
		
		@return List of managed crontab lines
		"""
		current_lines = self._get_current_crontab()
		managed_lines = []
		
		for line in current_lines:
			if self.MANAGED_MARKER in line:
				managed_lines.append(line)
		
		return managed_lines
	
	def _get_unmanaged_lines(self) -> List[str]:
		"""
		Get all crontab lines that are NOT managed by this tool.
		
		@return List of unmanaged crontab lines
		"""
		current_lines = self._get_current_crontab()
		unmanaged_lines = []
		
		for line in current_lines:
			if line.strip() and self.MANAGED_MARKER not in line:
				unmanaged_lines.append(line)
		
		return unmanaged_lines
	
	def install_schedule(self, schedule: Schedule) -> bool:
		"""
		Install a Schedule as a cron job.
		
		@param schedule The Schedule object to install
		@return True if successful, False otherwise
		"""
		try:
			cron_time = self._schedule_to_cron_time(schedule)
			cron_line = \
				f"{cron_time} {schedule.command}" + \
				f" {self.MANAGED_MARKER}:{schedule.name}"
			
			# Get existing lines, filter out old entry for this schedule
			unmanaged_lines = self._get_unmanaged_lines()
			managed_lines = self._get_managed_lines()
			
			# Remove any existing entry for this schedule name
			filtered_managed = [
				line for line in managed_lines 
				if not line.endswith(f":{schedule.name}")
			]
			
			# Add the new entry
			all_lines = unmanaged_lines + filtered_managed + [cron_line]
			
			return self._set_crontab(all_lines)
		except Exception:
			return False
	
	def remove_schedule(self, schedule_name: str) -> bool:
		"""
		Remove a scheduled task by name.
		
		@param schedule_name The name of the schedule to remove
		@return True if successful, False otherwise
		"""
		try:
			unmanaged_lines = self._get_unmanaged_lines()
			managed_lines = self._get_managed_lines()
			
			# Filter out the schedule to remove
			filtered_managed = [
				line for line in managed_lines 
				if not line.endswith(f":{schedule_name}")
			]
			
			all_lines = unmanaged_lines + filtered_managed
			
			return self._set_crontab(all_lines)
		except Exception:
			return False
	
	def list_schedules(self) -> List[str]:
		"""
		List all schedules managed by this backend.
		
		@return List of schedule names
		"""
		managed_lines = self._get_managed_lines()
		schedule_names = []
		
		for line in managed_lines:
			if ':' in line:
				# Extract schedule name from the marker
				marker_part = line.split(self.MANAGED_MARKER + ':')[-1]
				schedule_names.append(marker_part.strip())
		
		return schedule_names
	
	def clear_all_schedules(self) -> bool:
		"""
		Remove all schedules managed by this backend.
		
		@return True if successful, False otherwise
		"""
		try:
			unmanaged_lines = self._get_unmanaged_lines()
			return self._set_crontab(unmanaged_lines)
		except Exception:
			return False


class TaskScheduler:
	"""
	Main scheduler class that provides a unified interface for scheduling
	tasks across different platforms.
	
	This class:
	1. Detects the current platform
	2. Selects the appropriate backend
	3. Provides high-level scheduling operations
	"""
	
	def __init__(self):
		self.platform = self._detect_platform()
		self.backend = self._create_backend()
	
	def _detect_platform(self) -> PlatformType:
		"""
		Detect the current platform.
		
		@return The detected platform type
		"""
		system = platform.system().lower()
		
		if system == 'linux':
			return PlatformType.LINUX
		elif system == 'darwin':
			return PlatformType.DARWIN
		else:
			return PlatformType.UNKNOWN
	
	def _create_backend(self) -> SchedulerBackend:
		"""
		Create the appropriate backend for the detected platform.
		
		@return The scheduling backend instance
		"""
		if self.platform in [PlatformType.LINUX, PlatformType.DARWIN]:
			return CronBackend()
		else:
			raise RuntimeError(f"Unsupported platform: {self.platform}")

	def _check_schedule_conflicts(self, schedules: List[Schedule]) -> \
		Optional[Tuple[Schedule, Schedule]]:
		"""
		Check if any two schedules would run within 10 minutes of each other.
		
		@param schedules List of Schedule objects to check
		@return Tuple of conflicting schedules if found, None otherwise
		"""
		# For simplicity, we'll check conflicts within a 24-hour period
		# Convert each schedule to minutes from midnight for comparison
		
		def schedule_to_minutes_list(schedule: Schedule) -> List[int]:
			"""Convert schedule to list of minutes from midnight when it runs."""
			base_minutes = schedule.time.hour * 60 + schedule.time.minute
			minutes_list = []
			
			if schedule.stype == ScheduleType.EVERY_X_DAYS:
				# For every X days, it could run on any day, so check daily
				minutes_list.append(base_minutes)
			elif schedule.stype == ScheduleType.WEEKLY:
				# Weekly schedules run once per week, convert to daily equivalent
				# We'll check all 7 days to be safe
				for day in range(7):
					if day == (schedule.day % 7):  # The actual day it runs
						minutes_list.append(base_minutes)
			elif schedule.stype == ScheduleType.MONTHLY:
				# Monthly schedules run once per month, treat as daily for
				# conflict check
				minutes_list.append(base_minutes)
			
			return minutes_list
		
		# Get all execution times for each schedule
		schedule_times = []
		for schedule in schedules:
			times = schedule_to_minutes_list(schedule)
			for time_minutes in times:
				schedule_times.append((schedule, time_minutes))
		
		# Check for conflicts (within 10 minutes = 600 seconds)
		CONFLICT_THRESHOLD = 10  # minutes
		
		for i, (schedule1, time1) in enumerate(schedule_times):
			for j, (schedule2, time2) in enumerate(schedule_times[i+1:], 
													i+1):
				if schedule1.name == schedule2.name:
					continue  # Skip same schedule
				
				# Check if times are within 10 minutes
				time_diff = abs(time1 - time2)
				# Also check wrap-around (e.g., 23:55 and 00:05)
				time_diff_wrap = min(time_diff, 1440 - time_diff)  # 1440 = 24*60
				
				if time_diff_wrap <= CONFLICT_THRESHOLD:
					return (schedule1, schedule2)
		
		return None

	def install_config_schedules(self, config: Config) -> bool:
		"""
		Install all schedules from a Config object.
		
		@param config The Config object containing schedules
		@return True if all schedules installed successfully
		"""
		# Check for schedule conflicts before installing
		conflict = self._check_schedule_conflicts(config.schedules)
		if conflict:
			schedule1, schedule2 = conflict
			print(f"ERROR: Schedule conflict detected between " + \
				  f"'{schedule1.name}' and '{schedule2.name}'")
			print(f"  '{schedule1.name}' runs at " + \
				  f"{schedule1.time.strftime('%H:%M')}")
			print(f"  '{schedule2.name}' runs at " + \
				  f"{schedule2.time.strftime('%H:%M')}")
			print("  Schedules must be at least 10 minutes apart to " + \
				  "prevent database race conditions.")
			return False
	
		success = True
		
		for schedule in config.schedules:
			if not self.backend.install_schedule(schedule):
				print(f"Failed to install schedule: {schedule.name}")
				success = False
			else:
				print(f"Successfully installed schedule: {schedule.name}")
		
		return success
	
	def remove_schedule(self, schedule_name: str) -> bool:
		"""
		Remove a specific schedule by name.
		
		@param schedule_name The name of the schedule to remove
		@return True if successful
		"""
		return self.backend.remove_schedule(schedule_name)
	
	def list_schedules(self) -> List[str]:
		"""
		List all currently installed schedules.
		
		@return List of schedule names
		"""
		return self.backend.list_schedules()
	
	def clear_all_schedules(self) -> bool:
		"""
		Remove all schedules managed by this tool.
		
		@return True if successful
		"""
		return self.backend.clear_all_schedules()


def main():
	"""
	Main function demonstrating usage of the TaskScheduler.
	"""
	import sys
	
	if len(sys.argv) < 2:
		print(f"Usage: python3 {sys.argv[0]} <command> [args...]")
		print("Commands:")
		print("  install <config_path>    Install schedules from config")
		print("  remove <schedule_name>   Remove a specific schedule")
		print("  list                     List all managed schedules")
		print("  clear                    Remove all managed schedules")
		sys.exit(1)
	
	command = sys.argv[1]
	scheduler = TaskScheduler()
	
	if command == "install":
		if len(sys.argv) != 3:
			print("Usage: python scheduler.py install <config_path>")
			sys.exit(1)
		
		config_path = sys.argv[2]
		config = Config.load_from(config_path)
		
		if scheduler.install_config_schedules(config):
			print("All schedules installed successfully")
		else:
			print("Some schedules failed to install")
			sys.exit(1)
	
	elif command == "remove":
		if len(sys.argv) != 3:
			print("Usage: python scheduler.py remove <schedule_name>")
			sys.exit(1)
		
		schedule_name = sys.argv[2]
		if scheduler.remove_schedule(schedule_name):
			print(f"Schedule '{schedule_name}' removed successfully")
		else:
			print(f"Failed to remove schedule '{schedule_name}'")
			sys.exit(1)
	
	elif command == "list":
		schedules = scheduler.list_schedules()
		if schedules:
			print("Managed schedules:")
			for schedule in schedules:
				print(f"  - {schedule}")
		else:
			print("No managed schedules found")
	
	elif command == "clear":
		if scheduler.clear_all_schedules():
			print("All managed schedules cleared successfully")
		else:
			print("Failed to clear schedules")
			sys.exit(1)
	
	else:
		print(f"Unknown command: {command}")
		sys.exit(1)


if __name__ == "__main__":
	main()
