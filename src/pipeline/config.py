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
inherits from ConfigSection and has a virtual method to_dict().
You might ask, why don't I use dataclasses.asdict(). The problem is, fields of
certain types, like datetime.time, cannot be automatically handled by
JSON.dump.

Therefore, I must do manual handling after asdict.
"""

import datetime
import json
import os
# Python won't support returning the class itself from staticmethods!
from typing import Self
# No override in 3.11. But I have to use 3.11 for open-webui.
# from typing import override

from enum import Enum
from dataclasses import dataclass
import dataclasses


class ConfigSection:
	def to_dict(self) -> dict:
		"""
		One should call dataclasses.asdict() first,
		and then handle all fields that cannot be handled by json.dump()
		automatically.
		"""
		raise NotImplementedError("Abstract method")


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
Schedule_project_root : str = ""
@dataclass
class Schedule(ConfigSection):
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

	#@override
	def to_dict(self) -> dict:
		"""
		Need to convert datetime.time to str here.
		"""
		d = dataclasses.asdict(self)
		d["time"] = self.time.isoformat()

		return d

	# Because Python allows any type to be passed in in ctor,
	# I have to write this stupid __post_init__
	# How I miss C++.
	def __post_init__(self):
		if isinstance(self.stype, str):
			self.stype = ScheduleType(self.stype)
		if isinstance(self.time, str):
			self.time = datetime.time.fromisoformat(self.time)

	@staticmethod
	def project_root_path() -> str:
		"""
		@returns path to the root of the project. That is, where we have ./src
		and ./bin.
		"""
		global Schedule_project_root
		# Assumes the script can ever be run in project root.
		if (Schedule_project_root == ""):
			cwd = os.getcwd()
			if not cwd.endswith('/'):
				cwd += '/'
			Schedule_project_root = cwd

		return Schedule_project_root

	@staticmethod
	def user_home_path() -> str:
		"""
		@returns path to the user home dir.
		"""
		home = os.path.expanduser("~")
		if not home.endswith('/'):
			home += '/'
		return home

	@staticmethod
	def relative_cmd_prefix() -> str:
		"""
		Calculates the relative cmd prefix and caches it into
		Schedule_relative_cmd_prefix, since it won't change in one runtime
		session.
		"""
		return Schedule.project_root_path() + "bin/"


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
		self.command = Schedule.relative_cmd_prefix() + relcmd
			

@dataclass
class EmailInfo(ConfigSection):
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

	#@override
	def to_dict(self) -> dict:
		"""
		Nothing more to do here.
		"""
		return dataclasses.asdict(self)


@dataclass
class SearchConf(ConfigSection):
	system_prompt : str
	max_prompts : int

	#@override
	def to_dict(self) -> dict:
		"""
		Nothing more to do here.
		"""
		return dataclasses.asdict(self)


@dataclass
class SynthesisConf(ConfigSection):
	system_prompt : str
	max_len : int

	#@override
	def to_dict(self) -> dict:
		"""
		Nothing more to do here.
		"""
		return dataclasses.asdict(self)


CONFIG_DEF_SEARCH_GEN_RPOMPT : str = \
"""
You will receive abstract or concrete business-related concepts or
documents (text, images, etc.). Your task is to generate search engine
queries that would retrieve current or recent business news articles
discussing these concepts in action.

When the concept is abstract (e.g., "vertical integration", "global
value chain", "how to evaluation a company's value"), follow these
rules STRICTLY: 1. Identify concrete real-world examples, events,
company names, industries, or case studies that might illustrate the
concept. Use these to generate multiple keyword-based queries that
search engines can match easily. Always remind yourself that a search
engine can only do word match and cannot understand abstract ideas.  2.
The final search queries for an abstract concept, e.g., "vertical
integration" should include both the technical term, "vertical
integration", its synonyms, e.g. "vertical consolidation", and the
concrete phrases you think of. Connect the technical terms and concrete
terms with OR, not AND.  3. Both 1 and 2 must appear in a query
generated for an abstract input!  However, when the input itself is
concrete, then you don't have to deabstract it.

For example: (Abstract) Input: "Vertical integration" Output: 
		"vertical integration" OR "vertical consolidation" OR (Amazon
		warehouse logistics retail)

		"vertical integration" OR "vertical consolidation" OR (Tesla
		battery production) OR (vehicle manufacturing)

		"vertical integration" OR "vertical consolidation" OR (Apple
		chip design manufacturing)

		"vertical integration" OR "vertical consolidation" OR
		"Companies investing in end-to-end supply chains"

	(Abstract) Input: "Competitive advantage" Output: 
		"competitive advantage" OR "strategic edge" OR (Apple vs
		Microsoft battle)

		"competitive advantage" OR "strategic edge" OR (Microsoft's
		global dominance advantage)

	(Concrete) Input: "Trump tariff" Output: 
		Trump latest tariff

		Trump China tariff

		Trump tariff news

		Traiff Trump impacts.


You could use boolean opeartors like AND and OR, but be very careful
with AND, as it may lead not too narrow matches.

Only output search queries, separating them by newlines. Do not explain
or instruct. And DO NOT enclose the entire queries in e.g. quotes or
special symbols.
"""
CONFIG_DEF_SYN_PROMPT : str = \
"""
Forget ALL previous instructions!!!

You will be given 
1. a business topic.
2. a list of results from search engine that are about the
business topic. Each result will be a webpage url + tab + its title.

Your task is to:
1. Internally, rerank the results based on relevance to the given
topic, find the most relevant ones.
2. Summarize them, identify key articles from the results, and remember
to include the URLs.

You should always try to include the URLs.
"""
@dataclass
class Config:
	"""
	Represents a json config.

	@field business_topics the topics about each of which Business news will be
	searched.
	@field text_model the LLM model used to process text-only input.
	@field file_model the LLM model used to process text and files inputs.
	@field verbose_level controls how verbose the tool is.

	@field schedules each of which describes a schedule task that will run a
	command automatically at a specific time.
	@field email information related to automatic result delivery by email.
	"""
	# global entries in the JSON
	business_topics : list[str]
	text_model : str
	file_model : str
	verbose_level : int

	# configuration for stages in the pipeline 
	search_conf : SearchConf
	synthesis_conf : SynthesisConf

	# scheduled execution
	schedules : list[Schedule]

	# auto delivery by email
	email_info : EmailInfo

	# Because Python allows any type to be passed in in ctor,
	# I have to write this stupid __post_init__
	# How I miss C++.
	def __post_init__(self):
		if (isinstance(self.search_conf, dict)):
			self.search_conf = SearchConf(**self.search_conf)
		if (isinstance(self.synthesis_conf, dict)):
			self.synthesis_conf = SynthesisConf(**self.synthesis_conf)

		converted_sch = [
			Schedule(**s) if isinstance(s, dict)
			else s
			for s in self.schedules
		]
		self.schedules = converted_sch

		if (isinstance(self.email_info, dict)):
			self.email_info = EmailInfo(**self.email_info)

	@classmethod
	def load_default(cls : type[Self]) -> Self:
		business_topics : list[str] = [
			"Vertical integration in business",
			"Diversification strategies in business",
			"Competitive advantage in business",
			"Foreign direct investment in business"
		];
		text_model : str = "llama3.1:8b"
		file_model : str = "qwen2.5vl:7b"
		verbose_level : int = 1

		search_conf = SearchConf(
			system_prompt = \
				CONFIG_DEF_SEARCH_GEN_RPOMPT,
			max_prompts = 5
		)
		synthesis_conf = SynthesisConf(
			system_prompt = \
				CONFIG_DEF_SYN_PROMPT,
			max_len = 3200
		)

		schedules : list[Schedule] = [
			Schedule(
				name = "updater",
				stype = ScheduleType.EVERY_X_DAYS,
				day = 3,
				time = datetime.time(12,0),
				command = 
					"/bin/bash " + 
					Schedule.project_root_path() + 
					"run_updater.sh " + 
					Schedule.project_root_path() +
					' ' +
					Schedule.user_home_path(),
				catch_up = True
			),
			Schedule(
				name = "llm_pipeline",
				stype = ScheduleType.WEEKLY,
				day = 1,
				time = datetime.time(12,0),
				# May need to activate a few environments
				# before running the Python pipeline. Thus, I put the work
				# into one sh.
				command = 
					"/bin/bash " + 
					Schedule.project_root_path() +
					"run_pipeline.sh " +
					Schedule.project_root_path() +
					' ' +
					Schedule.user_home_path(),
				catch_up = True
			),
		]
		email_info = EmailInfo(
			# Since the code will be published,
			# the dst address will be my email address that can be known
			# publicly from my GitHub anyway.
			dst_addresses = [
				# Use my least valuable email address: the Microsoft one I
				# registered a long time ago and want to discard ever since.
				# But sadly I cannot as many part of the world requires me to
				# use Microsoft services in my daily life.
				"guanyuminghe@outlook.com"
			],
			# the src email address is a pure burner account.
			# It is used for nothing else except this.
			src_address = "anonytempburner@gmail.com",
			# Its password is a random base64 8 character string.
			src_passwd = "xfvJaGS8JpA=",
			src_provider = "smtp.gmail.com"
		)

		return cls(
			# global options
			business_topics,
			text_model,
			file_model,
			verbose_level,
			# stage conf
			search_conf,
			synthesis_conf,
			# scheduled execution
			schedules,
			# auto delivery by email
			email_info
		)


	@classmethod
	def from_dict(cls : type[Self], d : dict) -> Self:
		try:
			return cls(**d)
		except json.JSONDecodeError as e:
			raise RuntimeError(
				"Invalid config JSON"
			)

	@classmethod
	def load_from(cls : type[Self], path : str) -> Self:
		with open(path, 'r') as f:
			d : dict = json.load(f)
			return cls.from_dict(d)

	def save_to(self, path : str) -> None:
		"""
		I will avoid data race here by writing to a tmp file first
		and then using rename, which is atomic in most Unix-like systems.
		That is, there will be no point when the target file is presented in an
		incomplete state. It is either in its old state, or the new state.
		"""
		tmp_path = path + ".tmp"
		with open(tmp_path, 'w') as tmp:
			# Need to call each's to_dict() manually; so sad.
			# This is because dataclasses somehow decided to make 
			# asdict() a function of the module, not a virtual function of 
			# a dataclass class.
			d : dict = {}

			for fi in dataclasses.fields(self):
				attr = getattr(self, fi.name)
				# call custom to_dict for all ConfigSections
				if isinstance(attr, ConfigSection):
					d[fi.name] = attr.to_dict()
				elif isinstance(attr, list):
					l : list = [
						a.to_dict() if isinstance(a, ConfigSection)
						else a
						for a in attr
					]
					d[fi.name] = l
				else:
					d[fi.name] = attr


			json.dump(d, tmp, indent=4)

		# Python divides the traditional rename() sys call
		# into an os.rename() that will fail if dst exists
		# and an  os.replace() whose behaviour is unspecified. It may seem that
		# it will fail when dst does not exist, as the word "replace" implies
		# we are dealing with two objects, with dst to be replaced.
		#
		# Unfortunately, Python's doc for replace does not say anything about
		# what will happen when the file does not exist. And I can only test,
		# which shows that it will normally proceed when the file does not
		# exist. That is, replace works exactly like the normal rename syscall,
		# which is what I need here.
		os.replace(tmp_path, path)


