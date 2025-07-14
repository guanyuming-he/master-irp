"""
@author Guanyuming He
Copyright (C) Guanyuming He 2025
The file is licensed under the GNU GPL v3.

The file defines the Config class and related 
classes that encapsulates the access to the config.json
configuration file that controls the LLM pipeline.

Why do I abstract it away? Why don't I use a JSON object directly?

1. Correct today and correct in the unknown future:
	The class provides an encapsulation of the config structure inside the JSON
	file, without which every place that accesses the json needs to be changed
	whenever the structure change. That would be very error prone, as new
	entries may be added the the config every now and then.

	Moreover, when the writing and reading are centrally controlled by this
	class, I can prevent data race from happening more easily, when multiple
	programs are accessing it. The technique will likely be writing to a temp
	and then mv, I predict, even if I am not writing that yet.

2. Easy to understand:
	Clearly, it would be easier to understand, using well encapsulated methods
	in this class than using JSON names directly.

3. Ready for change:
	Similar to 1, when the structure of the config change, mostly I only need
	to change this class.


The Config class consists of a few fields each of which is an object of a
dataclass repesenting a specific section in the configuration. Each such class
can be converted to a dict by dataclass.asdict() 
"""

import datetime
import json
import os

from enum import Enum
from dataclasses import dataclass


class ScheduleType(str, Enum):
	"""
	Each value corresponds to the JSON entry 
	type.
	"""
	EVERY_X_DAYS = "every_x_days"
	WEEKLY = "weekly"
	MONTHLY = "monthly"


# dataclass fucks up the boundary between static and instance fields in a
# class. As such, I have to define a variable outside Schedule to act as its
# static field.
Schedule_relative_cmd_prefix : str = ""
@dataclass
class Schedule:
	"""
	config["schedules"] is a list of Schedules, each of which describes how a
	job is scheduled.

	json structure
	$name: {
		for attr except name in attributes
		"attr" : $attr,
	}

	@field name a unique name of the schedule task
	@field stype the type of the schedule. I cannot use the name type because
	it's reserved.
	@field day is interpreted based on stype:
		every_x_days: then day is the x.
		weekly: then day is 1--7
		montly: then day is 1--31
	@field time at what time in the specified day is the task executed. In
	JSON, then string will be in ISO format.
	@field command which command to run in sh.
	@field catch_up if the task should catch up if it was not fired at the last
	specified time for some reason.
	"""
	name : str
	stype : ScheduleType
	day : int
	time : datetime.time 
	command : str
	catch_up : bool

	@staticmethod
	def relative_cmd_prefix() -> str:
		"""
		Calculates the relative cmd prefix and caches it into
		Schedule_relative_cmd_prefix, since it won't change in one runtime
		session.
		"""
		if (Schedule_relative_cmd_prefix == ""):
			cwd = os.getcwd()
			if not cwd.endswith('/'):
				cwd += '/'

			Schedule_relative_cmd_prefix =cwd + "bin/"

		return Schedule_relative_cmd_prefix


	def relative_cmd_bin(self, relcmd: str) -> None:
		"""
		As binaries are compiled to a fixed location in the project directory,
		this methods turns relative binary paths to absolute ones in the
		command, by appending the binary dir path to it.

		After the call, self.command is set to it.

		@assumption Assume that every executable is always run from project
		root such that cwd = project root.

		@param relcmd the command whose first token is the relative binary
		path.
		"""
		self.command = relative_cmd_prefix() + relcmd
			

@dataclass
class EmailInfo:
	"""
	Information about automatic delivery of results 
	to user email addresses.

	The src address should be a burner one, because its password will be 
	exposed in the source code.
	"""
	dst_addresses : list[str]
	src_address : str
	src_passwd : str
	src_provider : str


@dataclass
class PromptGenConf:
	system_prompt : str
	max_prompts : int


@dataclass
class SynthesisConf:
	system_prompt : str
	max_len : int

class Config:
	"""
	Represents a json config.
	"""
	schedules : list[Schedule]



