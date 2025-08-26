"""
@author Guanyuming He
Copyright (C) Guanyuming He 2025
The file is licensed under the GNU GPL v3.

The file creates a GUI that helps the user to write to config.json.The GUI is
designed to be easily extensible - new config sections can be added by creating
new ConfigSectionFrame classes that inherit from the base ConfigSectionFrame.

This design, like that for my config related classes in ./config.py,
make the program
1. Correct today and in the unknown future:
	Each configuration section is isolated in GUI. One mistake won't mess up
	the others, and one change only requires modification in one place.
2. Easy to understand:
	Abstracting related config entries into one section makes the program easy
	to understand.
3. Ready for change:
	As mentioned earlier, it's designed for extensibility.

Because creating GUI elements is tedious and seldom a work that can improve
me a lot, I let LLMs help me write the tedious part, like building widgets.
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox, scrolledtext, font
import datetime
import json
import os
import base64
import threading
from typing import Any, Dict, List, Optional

# local imports
from config import (
	Config, ConfigSection, 
	Schedule, ScheduleType, 
	EmailInfo,
	SearchConf, SynthesisConf
)
from llm_interface import send_to_ollama
# Use the pipeline's default path
from llm_pipeline import DEFAULT_CONFIG_PATH
from config_gui_base import (
	DEFAULT_FONT,
	ConfigDialog,
	ConfigSectionFrame
)


class SearchConfFrame(ConfigSectionFrame):
	"""GUI frame for SearchConf configuration."""
	
	def __init__(self, parent: tk.Widget):
		super().__init__(parent, "Search Configuration")
		self.create_widgets()
		
	def create_widgets(self) -> None:
		# System prompt
		ttk.Label(self.frame, text="System Prompt:").pack(anchor='w')
		self.widgets['system_prompt_1'] = scrolledtext.ScrolledText(
			self.frame, height=10, width=80, wrap=tk.WORD,
			font=DEFAULT_FONT
		)
		self.widgets['system_prompt_1'].pack(fill='both', expand=True, pady=5)
		self.widgets['system_prompt_2'] = scrolledtext.ScrolledText(
			self.frame, height=10, width=80, wrap=tk.WORD,
			font=DEFAULT_FONT
		)
		self.widgets['system_prompt_2'].pack(fill='both', expand=True, pady=5)
		
		# Max prompts
		prompt_frame = ttk.Frame(self.frame)
		prompt_frame.pack(fill='x', pady=5)
		ttk.Label(prompt_frame, text="Max Prompts:").pack(side='left')
		self.widgets['max_subtopics'] = ttk.Spinbox(
			prompt_frame, from_=1, to=10, width=10
		)
		self.widgets['max_subtopics'].pack(side='left', padx=(10, 0))
		self.widgets['max_prompts_per_topic'] = ttk.Spinbox(
			prompt_frame, from_=1, to=10, width=10
		)
		self.widgets['max_prompts_per_topic'].pack(side='left', padx=(10, 0))

		# Google Search engine ID
		src_frame = ttk.Frame(self.frame)
		src_frame.pack(fill='x', pady=5)
		ttk.Label(src_frame, text="Google engine ID:").pack(side='left')
		self.widgets['google_id'] = ttk.Entry(src_frame, width=40)
		self.widgets['google_id'].pack(
			side='left', padx=(10, 0), fill='x', expand=True
		)

		# Google API key
		src_frame = ttk.Frame(self.frame)
		src_frame.pack(fill='x', pady=5)
		ttk.Label(src_frame, text="Google API key:").pack(side='left')
		self.widgets['google_api'] = ttk.Entry(src_frame, width=40)
		self.widgets['google_api'].pack(
			side='left', padx=(10, 0), fill='x', expand=True
		)

		
	def load_data(self, data: SearchConf) -> None:
		self.widgets['system_prompt_1'].delete('1.0', tk.END)
		self.widgets['system_prompt_1'].insert('1.0', data.system_prompt_1)
		self.widgets['system_prompt_2'].delete('1.0', tk.END)
		self.widgets['system_prompt_2'].insert('1.0', data.system_prompt_2)
		self.widgets['max_subtopics'].set(str(data.max_subtopics))
		self.widgets['max_prompts_per_topic'].set(str(data.max_prompts_per_topic))
		self.widgets['google_id'].delete(0, tk.END)
		self.widgets['google_id'].insert(0, data.google_engine_id)
		self.widgets['google_api'].delete(0, tk.END)
		self.widgets['google_api'].insert(0, data.google_api_key)
		
	def get_data(self) -> SearchConf:
		return SearchConf(
			system_prompt_1=self.widgets['system_prompt_1'].get('1.0', tk.END).strip(),
			system_prompt_2=self.widgets['system_prompt_2'].get('1.0', tk.END).strip(),
			max_subtopics=int(self.widgets['max_subtopics'].get()),
			max_prompts_per_topic=int(self.widgets['max_prompts_per_topic'].get()),
			google_engine_id=self.widgets['google_id'].get(),
			google_api_key=self.widgets['google_api'].get(),
		)


class SynthesisConfFrame(ConfigSectionFrame):
	"""GUI frame for SynthesisConf configuration."""
	
	def __init__(self, parent: tk.Widget):
		super().__init__(parent, "Synthesis Configuration")
		self.create_widgets()
		
	def create_widgets(self) -> None:
		# System prompt
		ttk.Label(self.frame, text="System Prompt:").pack(anchor='w')
		self.widgets['system_prompt_1'] = scrolledtext.ScrolledText(
			self.frame, height=10, width=80, wrap=tk.WORD,
			font=DEFAULT_FONT
		)
		self.widgets['system_prompt_1'].pack(fill='both', expand=True, pady=5)
		self.widgets['system_prompt_2'] = scrolledtext.ScrolledText(
			self.frame, height=10, width=80, wrap=tk.WORD,
			font=DEFAULT_FONT
		)
		self.widgets['system_prompt_2'].pack(fill='both', expand=True, pady=5)
		
		# Max length
		len_frame = ttk.Frame(self.frame)
		len_frame.pack(fill='x', pady=5)
		ttk.Label(len_frame, text="Max Length:").pack(side='left')
		self.widgets['max_len'] = ttk.Spinbox(
			len_frame, from_=100, to=10000, width=10
		)
		self.widgets['max_len'].pack(side='left', padx=(10, 0))
		
	def load_data(self, data: SynthesisConf) -> None:
		self.widgets['system_prompt_1'].delete('1.0', tk.END)
		self.widgets['system_prompt_1'].insert('1.0', data.system_prompt_1)
		self.widgets['system_prompt_2'].delete('1.0', tk.END)
		self.widgets['system_prompt_2'].insert('1.0', data.system_prompt_2)
		self.widgets['max_len'].set(str(data.max_len))
		
	def get_data(self) -> SynthesisConf:
		return SynthesisConf(
			system_prompt_1=self.widgets['system_prompt_1'].get('1.0', tk.END).strip(),
			system_prompt_2=self.widgets['system_prompt_2'].get('1.0', tk.END).strip(),
			max_len=int(self.widgets['max_len'].get())
		)


class ScheduleFrame(ConfigSectionFrame):
	"""GUI frame for Schedule configuration."""
	
	def __init__(self, parent: tk.Widget):
		super().__init__(parent, "Schedules")
		self.schedule_list : list[Schedule] = []
		self.create_widgets()
		
	def create_widgets(self) -> None:
		# Schedule list
		list_frame = ttk.Frame(self.frame)
		list_frame.pack(fill='both', expand=True, pady=5)
		
		ttk.Label(list_frame, text="Schedules:").pack(anchor='w')
		
		# Listbox with scrollbar
		listbox_frame = ttk.Frame(list_frame)
		listbox_frame.pack(fill='both', expand=True)
		
		self.widgets['listbox'] = tk.Listbox(listbox_frame, height=6)
		scrollbar = ttk.Scrollbar(listbox_frame, orient='vertical')
		self.widgets['listbox'].config(yscrollcommand=scrollbar.set)
		scrollbar.config(command=self.widgets['listbox'].yview)
		
		self.widgets['listbox'].pack(side='left', fill='both', expand=True)
		scrollbar.pack(side='right', fill='y')
		
		# Buttons
		btn_frame = ttk.Frame(list_frame)
		btn_frame.pack(fill='x', pady=5)
		
		ttk.Button(btn_frame, text="Add", 
				  command=self.add_schedule).pack(side='left', padx=5)
		ttk.Button(btn_frame, text="Edit", 
				  command=self.edit_schedule).pack(side='left', padx=5)
		ttk.Button(btn_frame, text="Delete", 
				  command=self.delete_schedule).pack(side='left', padx=5)
		
	def add_schedule(self) -> None:
		"""Add a new schedule."""
		dialog = ScheduleDialog(self.parent, "Add Schedule")
		if dialog.result:
			self.schedule_list.append(dialog.result)
			self.refresh_listbox()
			
	def edit_schedule(self) -> None:
		"""Edit selected schedule."""
		selection = self.widgets['listbox'].curselection()
		if not selection:
			messagebox.showwarning("Warning", "Please select a schedule to edit.")
			return
			
		index = selection[0]
		dialog = ScheduleDialog(self.parent, "Edit Schedule", 
								self.schedule_list[index])
		if dialog.result:
			self.schedule_list[index] = dialog.result
			self.refresh_listbox()
			
	def delete_schedule(self) -> None:
		"""Delete selected schedule."""
		selection = self.widgets['listbox'].curselection()
		if not selection:
			messagebox.showwarning("Warning", "Please select a schedule to delete.")
			return
			
		index = selection[0]
		if messagebox.askyesno("Confirm", "Delete selected schedule?"):
			del self.schedule_list[index]
			self.refresh_listbox()
			
	def refresh_listbox(self) -> None:
		"""Refresh the listbox display."""
		self.widgets['listbox'].delete(0, tk.END)
		for schedule in self.schedule_list:
			display_text = f"{schedule.name} - {schedule.stype.value} - {schedule.time}"
			self.widgets['listbox'].insert(tk.END, display_text)
			
	def load_data(self, data: List[Schedule]) -> None:
		self.schedule_list = data.copy()
		self.refresh_listbox()
		
	def get_data(self) -> List[Schedule]:
		return self.schedule_list.copy()


class ScheduleDialog(ConfigDialog):
	"""Dialog for editing individual schedules."""
	
	def __init__(
		self, 
		parent: tk.Widget, title: str, 
		schedule: Optional[Schedule] = None
	):
		self.result : Optional[Schedule] = None
		self.name_var = tk.StringVar()
		self.type_var = tk.StringVar()
		self.day_var = tk.StringVar()
		self.hour_var = tk.StringVar()
		self.minute_var = tk.StringVar()
		self.command_var = tk.StringVar()
		self.catch_up_var = tk.BooleanVar()
		if schedule:
			self.load_schedule(schedule)
			
		super().__init__(
			parent,
			600, 800,
			"Edit this scheduled task"
		)
		
	def create_custom_widgets(self) -> None:
		# Name
		ttk.Label(
			self.inner_frame, text="Name:"
		).grid(row=0, column=0, sticky='w', pady=5)
		ttk.Entry(self.inner_frame, textvariable=self.name_var, width=30).grid(
			row=0, column=1, sticky='ew', pady=5, padx=(10, 0)
		)
		
		# Schedule type
		ttk.Label(
			self.inner_frame, text="Type:"
		).grid(row=1, column=0, sticky='w', pady=5)
		type_combo = ttk.Combobox(
			self.inner_frame, textvariable=self.type_var, width=27
		)
		type_combo['values'] = [e.value for e in ScheduleType]
		type_combo.grid(row=1, column=1, sticky='ew', pady=5, padx=(10, 0))
		
		# Day
		ttk.Label(
			self.inner_frame, text="Day:"
		).grid(row=2, column=0, sticky='w', pady=5)
		ttk.Spinbox(self.inner_frame, textvariable=self.day_var, from_=1, to=31, 
				   width=30).grid(row=2, column=1, sticky='ew', pady=5, padx=(10, 0))
		
		# Time
		ttk.Label(self.inner_frame, text="Time (HH:MM):").grid(
			row=3, column=0, sticky='w', pady=5
		)
		time_frame = ttk.Frame(self.inner_frame)
		time_frame.grid(row=3, column=1, sticky='ew', pady=5, padx=(10, 0))
		
		
		ttk.Spinbox(time_frame, textvariable=self.hour_var, from_=0, to=23, 
				   width=5).pack(side='left')
		ttk.Label(time_frame, text=":").pack(side='left')
		ttk.Spinbox(time_frame, textvariable=self.minute_var, from_=0, to=59, 
				   width=5).pack(side='left')
		
		# Command
		ttk.Label(
			self.inner_frame, text="Command:"
		).grid(row=4, column=0, sticky='w', pady=5)
		ttk.Entry(self.inner_frame, textvariable=self.command_var, width=30).grid(
			row=4, column=1, sticky='ew', pady=5, padx=(10, 0)
		)
		
		# Catch up
		ttk.Checkbutton(self.inner_frame, text="Catch up", 
						variable=self.catch_up_var).grid(
			row=5, column=0, columnspan=2, sticky='w', pady=5
		)
		
		# Buttons
		btn_frame = ttk.Frame(self.inner_frame)
		btn_frame.grid(row=6, column=0, columnspan=2, pady=20)
		
		ttk.Button(btn_frame, text="OK", command=self.ok_clicked).pack(
			side='left', padx=5
		)
		ttk.Button(btn_frame, text="Cancel", command=self.cancel_clicked).pack(
			side='left', padx=5
		)
		
		# Configure grid weights
		self.inner_frame.columnconfigure(1, weight=1)
		
	def load_schedule(self, schedule: Schedule) -> None:
		"""Load schedule data into widgets."""
		self.name_var.set(schedule.name)
		self.type_var.set(schedule.stype.value)
		self.day_var.set(str(schedule.day))
		self.hour_var.set(str(schedule.time.hour))
		self.minute_var.set(str(schedule.time.minute))
		self.command_var.set(schedule.command)
		self.catch_up_var.set(schedule.catch_up)
		
	def ok_clicked(self) -> None:
		try:
			name = self.name_var.get().strip()
			if not name:
				messagebox.showerror("Error", "Name is required.")
				return
				
			stype = ScheduleType(self.type_var.get())
			day = int(self.day_var.get())
			hour = int(self.hour_var.get())
			minute = int(self.minute_var.get())
			command = self.command_var.get().strip()
			catch_up = self.catch_up_var.get()
			
			if not command:
				messagebox.showerror("Error", "Command is required.")
				return
				
			time = datetime.time(hour, minute)
			
			self.result = Schedule(
				name=name,
				stype=stype,
				day=day,
				time=time,
				command=command,
				catch_up=catch_up
			)
			
			self.dialog.destroy()
			
		except ValueError as e:
			messagebox.showerror("Error", f"Invalid input: {str(e)}")
			
	def cancel_clicked(self) -> None:
		self.dialog.destroy()


class EmailInfoFrame(ConfigSectionFrame):
	"""GUI frame for EmailInfo configuration."""
	
	def __init__(self, parent: tk.Widget):
		super().__init__(parent, "Email Information")
		self.create_widgets()
		
	def create_widgets(self) -> None:
		# Destination addresses
		ttk.Label(self.frame, text="Destination Addresses:").pack(anchor='w')
		self.widgets['dst_addresses'] = scrolledtext.ScrolledText(
			self.frame, height=4, width=80,
			font=DEFAULT_FONT
		)
		self.widgets['dst_addresses'].pack(fill='x', pady=5)
		ttk.Label(self.frame, text="(One email per line)", 
				 font=('TkDefaultFont', 8)).pack(anchor='w')
		
		# Source address
		src_frame = ttk.Frame(self.frame)
		src_frame.pack(fill='x', pady=5)
		ttk.Label(src_frame, text="Source Address:").pack(side='left')
		self.widgets['src_address'] = ttk.Entry(src_frame, width=40)
		self.widgets['src_address'].pack(side='left', padx=(10, 0), fill='x', expand=True)
		
		# Source password
		passwd_frame = ttk.Frame(self.frame)
		passwd_frame.pack(fill='x', pady=5)
		ttk.Label(passwd_frame, text="Source Password:").pack(side='left')
		self.widgets['src_passwd'] = ttk.Entry(passwd_frame, width=40, show='*')
		self.widgets['src_passwd'].pack(side='left', padx=(10, 0), fill='x', expand=True)
		
		# Source provider
		provider_frame = ttk.Frame(self.frame)
		provider_frame.pack(fill='x', pady=5)
		ttk.Label(provider_frame, text="Source Provider:").pack(side='left')
		self.widgets['src_provider'] = ttk.Entry(provider_frame, width=40)
		self.widgets['src_provider'].pack(side='left', padx=(10, 0), fill='x', expand=True)
		
	def load_data(self, data: EmailInfo) -> None:
		# Load destination addresses
		self.widgets['dst_addresses'].delete('1.0', tk.END)
		if data.dst_addresses:
			self.widgets['dst_addresses'].insert('1.0', '\n'.join(data.dst_addresses))
			
		# Load other fields
		self.widgets['src_address'].delete(0, tk.END)
		self.widgets['src_address'].insert(0, data.src_address)
		
		self.widgets['src_passwd'].delete(0, tk.END)
		self.widgets['src_passwd'].insert(0, data.src_passwd)
		
		self.widgets['src_provider'].delete(0, tk.END)
		self.widgets['src_provider'].insert(0, data.src_provider)
		
	def get_data(self) -> EmailInfo:
		# Get destination addresses
		dst_text = self.widgets['dst_addresses'].get('1.0', tk.END).strip()
		dst_addresses = [addr.strip() for addr in dst_text.split('\n') 
						if addr.strip()]
		
		return EmailInfo(
			dst_addresses=dst_addresses,
			src_address=self.widgets['src_address'].get(),
			src_passwd=self.widgets['src_passwd'].get(),
			src_provider=self.widgets['src_provider'].get()
		)


class ConfigGUI:
	"""Main GUI application for configuration management."""
	
	def __init__(self, config_path : str):
		self.root = tk.Tk()
		self.root.title("LLM Pipeline Configuration")
		self.root.geometry("900x700")
		
		# Config global font style for all ttk widgets.
		style = ttk.Style(self.root)
		style.configure('.', font=DEFAULT_FONT)
		for cls in [
			"TLabel", "TButton", "TEntry", 
			"TCombobox", "TSpinbox"
		]:
			style.configure(cls, font=DEFAULT_FONT)

		self.config : Optional[Config] = None
		self.current_file : Optional[str] = config_path
		
		self.create_widgets()
		self.load_config(config_path)
		
	def create_widgets(self) -> None:
		"""Create the main GUI widgets."""
		# Menu bar
		self.create_menu()
		
		# Main content area with scrolling
		self.canvas = tk.Canvas(self.root)
		self.scrollbar = ttk.Scrollbar(self.root, orient="vertical", 
									  command=self.canvas.yview)
		self.scrollable_frame = ttk.Frame(self.canvas)
		
		self.scrollable_frame.bind(
			"<Configure>",
			lambda e: self.canvas.configure(scrollregion=self.canvas.bbox("all"))
		)
		
		self.canvas.create_window((0, 0), window=self.scrollable_frame, anchor="nw")
		self.canvas.configure(yscrollcommand=self.scrollbar.set)
		
		# Pack scrolling components
		self.canvas.pack(side="left", fill="both", expand=True)
		self.scrollbar.pack(side="right", fill="y")
		
		# Bind mousewheel to canvas
		self.canvas.bind_all("<MouseWheel>", self._on_mousewheel)
		
		# Create config sections
		self.create_config_sections()
		
	def _on_mousewheel(self, event) -> None:
		"""Handle mouse wheel scrolling."""
		self.canvas.yview_scroll(int(-1*(event.delta/120)), "units")
		
	def create_menu(self) -> None:
		"""Create the menu bar."""
		menubar = tk.Menu(self.root)
		self.root.config(menu=menubar)
		
		# File menu
		file_menu = tk.Menu(menubar, tearoff=0)
		menubar.add_cascade(label="File", menu=file_menu)
		file_menu.add_command(label="New", command=self.new_config)
		file_menu.add_command(label="Open", command=self.open_config)
		file_menu.add_command(label="Save", command=self.save_config)
		file_menu.add_command(label="Save As", command=self.save_config_as)
		file_menu.add_separator()
		file_menu.add_command(label="Exit", command=self.root.quit)
		
		# Help menu
		help_menu = tk.Menu(menubar, tearoff=0)
		menubar.add_cascade(label="Help", menu=help_menu)
		help_menu.add_command(label="About", command=self.show_about)
		
	def create_config_sections(self) -> None:
		"""Create all configuration section frames."""
		# Global settings
		self.create_global_settings()
		
		# Search configuration
		self.search_frame = SearchConfFrame(self.scrollable_frame)
		self.search_frame.pack(fill='both', expand=True, padx=10, pady=5)
		
		# Synthesis configuration
		self.synthesis_frame = SynthesisConfFrame(self.scrollable_frame)
		self.synthesis_frame.pack(fill='both', expand=True, padx=10, pady=5)
		
		# Schedules
		self.schedule_frame = ScheduleFrame(self.scrollable_frame)
		self.schedule_frame.pack(fill='both', expand=True, padx=10, pady=5)
		
		# Email info
		self.email_frame = EmailInfoFrame(self.scrollable_frame)
		self.email_frame.pack(fill='both', expand=True, padx=10, pady=5)
		
	def create_global_settings(self) -> None:
		"""Create global settings frame."""
		global_frame = ttk.LabelFrame(self.scrollable_frame, text="Global Settings", 
									 padding="10")
		global_frame.pack(fill='x', padx=10, pady=5)
		
		# Business topics section
		self.create_business_topics_section(global_frame)
		
		# Models and verbose level
		model_frame = ttk.Frame(global_frame)
		model_frame.pack(fill='x', pady=5)
		
		ttk.Label(model_frame, text="Text Model:").grid(
			row=0, column=0, sticky='w', pady=2
		)
		self.text_model_var = tk.StringVar()
		ttk.Entry(
			model_frame, textvariable=self.text_model_var, width=30,
			font=DEFAULT_FONT
		).grid(
			row=0, column=1, sticky='ew', padx=(10, 0), pady=2
		)
		
		ttk.Label(model_frame, text="File Model:").grid(
			row=1, column=0, sticky='w', pady=2
		)
		self.file_model_var = tk.StringVar()
		ttk.Entry(
			model_frame, textvariable=self.file_model_var, width=30,
			font=DEFAULT_FONT
		).grid(
			row=1, column=1, sticky='ew', padx=(10, 0), pady=2
		)
		
		ttk.Label(model_frame, text="Verbose Level:").grid(
			row=2, column=0, sticky='w', pady=2
		)
		self.verbose_var = tk.StringVar()
		ttk.Spinbox(
			model_frame, textvariable=self.verbose_var, 
			from_=0, to=5, 
			width=30
		).grid(row=2, column=1, sticky='ew', padx=(10, 0), pady=2)
		
		model_frame.columnconfigure(1, weight=1)
		
	def create_business_topics_section(self, parent: tk.Widget) -> None:
		"""Create the business topics section with LLM generation."""
		# Business topics header
		topics_header = ttk.Frame(parent)
		topics_header.pack(fill='x', pady=(0, 5))
		
		ttk.Label(topics_header, text="Business Topics:").pack(side='left')
		ttk.Button(topics_header, text="Generate with LLM", 
				  command=self.open_llm_generator).pack(side='right')
		
		# Business topics text area
		self.business_topics_text = scrolledtext.ScrolledText(
			parent, height=6, width=80,
			font=DEFAULT_FONT
		)
		self.business_topics_text.pack(fill='x', pady=5)
		ttk.Label(parent, text="(One topic per line)", 
				 font=('TkDefaultFont', 8)).pack(anchor='w')
		
	def open_llm_generator(self) -> None:
		LLMTopicGenerator(
			self.root, self.config, self.on_topics_generated
		)
		
	def on_topics_generated(self, topics: List[str]) -> None:
		"""Handle generated topics from LLM."""
		if topics:
			# Ask user if they want to replace or append
			choice = messagebox.askyesnocancel(
				"Generated Topics",
				f"Generated {len(topics)} topics. Replace current topics?",
				icon='question'
			)
			
			if choice is True:  # Replace
				self.business_topics_text.delete('1.0', tk.END)
				self.business_topics_text.insert('1.0', '\n'.join(topics))
			elif choice is False:  # Append
				current_text = self.business_topics_text.get('1.0', tk.END).strip()
				if current_text:
					new_text = current_text + '\n' + '\n'.join(topics)
				else:
					new_text = '\n'.join(topics)
				self.business_topics_text.delete('1.0', tk.END)
				self.business_topics_text.insert('1.0', new_text)
		
	def load_config(self, config_path : str) -> None:
		"""
		Load config from config_path if it exists.
		Otherwise, load default config.
		"""
		if (os.path.exists(config_path)):
			self.config = Config.load_from(config_path)
		else:
			self.config = Config.load_default()

		self.load_config_to_gui()
		
	def load_config_to_gui(self) -> None:
		"""Load config data into GUI widgets."""
		if not self.config:
			return
			
		# Global settings
		self.business_topics_text.delete('1.0', tk.END)
		self.business_topics_text.insert(
			'1.0', '\n'.join(self.config.business_topics)
		)
		
		self.text_model_var.set(self.config.text_model)
		self.file_model_var.set(self.config.file_model)
		self.verbose_var.set(str(self.config.verbose_level))
		
		# Section frames
		self.search_frame.load_data(self.config.search_conf)
		self.synthesis_frame.load_data(self.config.synthesis_conf)
		self.schedule_frame.load_data(self.config.schedules)
		self.email_frame.load_data(self.config.email_info)
		
	def get_config_from_gui(self) -> Config:
		"""Extract configuration from GUI widgets."""
		# Global settings
		topics_text = self.business_topics_text.get('1.0', tk.END).strip()
		business_topics = [topic.strip() for topic in topics_text.split('\n') 
						  if topic.strip()]
		
		return Config(
			business_topics=business_topics,
			text_model=self.text_model_var.get(),
			file_model=self.file_model_var.get(),
			verbose_level=int(self.verbose_var.get()),
			search_conf=self.search_frame.get_data(),
			synthesis_conf=self.synthesis_frame.get_data(),
			schedules=self.schedule_frame.get_data(),
			email_info=self.email_frame.get_data()
		)
		
	def new_config(self) -> None:
		"""Create a new configuration."""
		if messagebox.askyesno("New Configuration", 
							  "Create new configuration? Unsaved changes will be lost."):
			self.config = Config.load_default()
			self.current_file = None
			self.load_config_to_gui()
			self.root.title("LLM Pipeline Configuration - New")
			
	def open_config(self) -> None:
		"""Open a configuration file."""
		filename = filedialog.askopenfilename(
			title="Open Configuration",
			filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
		)
		
		if filename:
			try:
				self.config = Config.load_from(filename)
				self.current_file = filename
				self.load_config_to_gui()
				self.root.title(
					f"LLM Pipeline Configuration "
					f"- {os.path.basename(filename)}"
				)
			except Exception as e:
				messagebox.showerror("Error", f"Failed to open config: {str(e)}")
				
	def save_config(self) -> None:
		"""Save the current configuration."""
		if self.current_file:
			try:
				config = self.get_config_from_gui()
				config.save_to(self.current_file)
				messagebox.showinfo("Success", "Configuration saved successfully.")
			except Exception as e:
				messagebox.showerror("Error", f"Failed to save config: {str(e)}")
		else:
			self.save_config_as()
			
	def save_config_as(self) -> None:
		"""Save configuration with a new filename."""
		filename = filedialog.asksaveasfilename(
			title="Save Configuration As",
			filetypes=[("JSON files", "*.json"), ("All files", "*.*")],
			defaultextension=".json"
		)
		
		if filename:
			try:
				config = self.get_config_from_gui()
				config.save_to(filename)
				self.current_file = filename
				self.root.title(
					f"LLM Pipeline Configuration"
					f" - {os.path.basename(filename)}"
				)
				messagebox.showinfo("Success", "Configuration saved successfully.")
			except Exception as e:
				messagebox.showerror("Error", f"Failed to save config: {str(e)}")
				
	def show_about(self) -> None:
		messagebox.showinfo("About", 
		   "LLM Pipeline Configuration Tool\n"
		   "Part of my IRP software\n"
		   "Copyright (C) Guanyuming He 2025\n"
		   "Licensed under GNU GPL v3"
		)
		
	def run(self) -> None:
		"""Run the GUI application."""
		self.root.mainloop()


class LLMTopicGenerator(ConfigDialog):
	"""Dialog for generating business topics using LLM."""
	
	def __init__(
		self, 
		parent: tk.Widget, config: Config, callback
	):
		self.config = config
		self.callback = callback
		# List of dicts of structure
		# {
		# 	'path': filename,
		# 	'name': os.path.basename(filename),
		# 	'encoded': encoded_data,
		# 	'size': len(file_data)
		# }
		self.attached_files : list[dict] = []
		self.result_topics : list[str] = []

		super().__init__(
			parent,
			600, 800,
			"Generate Business Topics with LLM"
		)
		
	def create_custom_widgets(self) -> None:
		"""Create dialog widgets."""
		
		# Instructions
		instruction_frame = ttk.LabelFrame(
			self.inner_frame, text="Instructions", 
			padding="10"
		)
		instruction_frame.pack(fill='x', pady=(0, 10))
		
		ttk.Label(instruction_frame, text="System Prompt:").pack(anchor='w')
		self.system_var = tk.StringVar()
		system_entry = ttk.Entry(
			instruction_frame, textvariable=self.system_var, 
			width=80,
			font=DEFAULT_FONT
		)
		system_entry.pack(fill='x', pady=(5, 10))
		self.system_var.set(
			"Generate business topics based on the input provided."
			" Do NOT explain or instruct. "
			" Put each topic in one line, and do not"
			" enclose with quotes or parentheses."
			" Do not generate too many."
			" Generate a few representative topics."
		)
		
		ttk.Label(instruction_frame, text="User Prompt:").pack(anchor='w')
		self.user_prompt_text = scrolledtext.ScrolledText(
			instruction_frame, height=8, width=70, wrap=tk.WORD,
			font=DEFAULT_FONT
		)
		self.user_prompt_text.pack(fill='both', expand=True, pady=5)
		
		# File attachments
		file_frame = ttk.LabelFrame(self.inner_frame, text="File Attachments", 
								   padding="10")
		file_frame.pack(fill='x', pady=(0, 10))
		
		# File list and buttons
		file_list_frame = ttk.Frame(file_frame)
		file_list_frame.pack(fill='x', pady=5)
		
		self.file_listbox = tk.Listbox(file_list_frame, height=4)
		file_scrollbar = ttk.Scrollbar(file_list_frame, orient='vertical')
		self.file_listbox.config(yscrollcommand=file_scrollbar.set)
		file_scrollbar.config(command=self.file_listbox.yview)
		
		self.file_listbox.pack(side='left', fill='both', expand=True)
		file_scrollbar.pack(side='right', fill='y')
		
		file_btn_frame = ttk.Frame(file_frame)
		file_btn_frame.pack(fill='x', pady=5)
		
		ttk.Button(file_btn_frame, text="Add Files", 
				  command=self.add_files).pack(side='left', padx=5)
		ttk.Button(file_btn_frame, text="Remove Selected", 
				  command=self.remove_selected_file).pack(side='left', padx=5)
		ttk.Button(file_btn_frame, text="Clear All", 
				  command=self.clear_files).pack(side='left', padx=5)
		
		# Generation controls
		gen_frame = ttk.Frame(self.inner_frame)
		gen_frame.pack(fill='x', pady=(0, 10))
		
		ttk.Label(gen_frame, text="Model:").pack(side='left')
		self.model_var = tk.StringVar()
		model_combo = ttk.Combobox(gen_frame, textvariable=self.model_var, 
								  width=20)
		model_combo.pack(side='left', padx=(5, 20))
		# Set default model based on whether files are attached
		self.model_var.set(self.config.file_model if self.config else "qwen2.5vl:7b")
		
		ttk.Button(gen_frame, text="Generate Topics", 
				  command=self.generate_topics).pack(side='left', padx=5)
		
		# Progress bar
		self.progress = ttk.Progressbar(gen_frame, mode='indeterminate')
		self.progress.pack(side='right', fill='x', expand=True, padx=(20, 0))
		
		# Results
		result_frame = ttk.LabelFrame(self.inner_frame, text="Generated Topics", 
									 padding="10")
		result_frame.pack(fill='both', expand=True, pady=(0, 10))
		
		self.result_text = scrolledtext.ScrolledText(
			result_frame, height=8, width=70, wrap=tk.WORD,
			font=DEFAULT_FONT
		)
		self.result_text.pack(fill='both', expand=True, pady=5)
		
		# Dialog buttons
		btn_frame = ttk.Frame(self.inner_frame)
		btn_frame.pack(fill='x')
		
		ttk.Button(btn_frame, text="Use Topics", 
				  command=self.use_topics).pack(side='right', padx=5)
		ttk.Button(btn_frame, text="Cancel", 
				  command=self.cancel).pack(side='right', padx=5)
		
		# Update model selection when files change
		self.file_listbox.bind('<<ListboxSelect>>', self.update_model_selection)
		
	def add_files(self) -> None:
		"""Add files to the attachment list."""
		filetypes = [
			("Images", "*.jpg *.jpeg *.png *.gif *.bmp *.tiff *.webp"),
			# unsupported for now
			# ("Documents", "*.txt *.pdf *.doc *.docx *.rtf"),
			# ("All files", "*.*")
		]
		
		filenames = filedialog.askopenfilenames(
			title="Select Files to Attach",
			filetypes=filetypes
		)
		
		for filename in filenames:
			if filename not in [f['path'] for f in self.attached_files]:
				try:
					# Read and encode file
					with open(filename, 'rb') as f:
						file_data = f.read()
					
					encoded_data = base64.b64encode(file_data).decode('utf-8')
					
					file_info = {
						'path': filename,
						'name': os.path.basename(filename),
						'encoded': encoded_data,
						'size': len(file_data)
					}
					
					self.attached_files.append(file_info)
					self.file_listbox.insert(tk.END, 
						f"{file_info['name']} ({file_info['size']} bytes)")
					
				except Exception as e:
					messagebox.showerror("Error", 
						f"Failed to load file {filename}: {str(e)}")
		
		self.update_model_selection()
		
	def remove_selected_file(self) -> None:
		"""Remove selected file from attachments."""
		selection = self.file_listbox.curselection()
		if selection:
			index = selection[0]
			del self.attached_files[index]
			self.file_listbox.delete(index)
			self.update_model_selection()
		
	def clear_files(self) -> None:
		"""Clear all attached files."""
		self.attached_files.clear()
		self.file_listbox.delete(0, tk.END)
		self.update_model_selection()
		
	def update_model_selection(self, event=None) -> None:
		"""Update model selection based on file attachments."""
		if self.attached_files:
			# Use file model when files are attached
			self.model_var.set(self.config.file_model if self.config else "qwen2.5vl:7b")
		else:
			# Use text model when no files
			self.model_var.set(self.config.text_model if self.config else "llama3.1:8b")
		
	def generate_topics(self) -> None:
		"""Generate topics using LLM."""
		user_prompt = self.user_prompt_text.get('1.0', tk.END).strip()
		
		if not user_prompt and not self.attached_files:
			messagebox.showwarning("Warning", 
				"Please provide either a text prompt or attach files.")
			return
		
		# Start generation in background thread
		self.progress.start()
		self.result_text.delete('1.0', tk.END)
		self.result_text.insert(tk.END, "Generating topics...\n")
		
		thread = threading.Thread(target=self._generate_topics_thread, 
								 args=(user_prompt,))
		thread.daemon = True
		thread.start()
		
	def _generate_topics_thread(self, user_prompt: str) -> None:
		"""Background thread for LLM topic generation."""
		try:
			# Prepare images for Ollama
			images = [f['encoded'] for f in self.attached_files]
			
			# Generate topics
			response = send_to_ollama(
				model=self.model_var.get(),
				system=self.system_var.get(),
				prompt=\
					user_prompt + \
					"\nGenerate LESS THAN 8 TOPICS!!!",
				images=images
			)
			
			# Parse topics from response
			topics = self._parse_topics_from_response(response)
			
			# Update UI in main thread
			self.dialog.after(0, self._update_results, topics, response)
			
		except Exception as e:
			error_msg = f"Error generating topics: {str(e)}"
			self.dialog.after(0, self._update_results, [], error_msg)
			
	def _parse_topics_from_response(self, response: str) -> List[str]:
		"""Parse business topics from LLM response."""
		topics = []
		lines = response.strip().split('\n')
		
		for line in lines:
			line = line.strip()
			if line:
				# Remove common prefixes like "1.", "-", "*", etc.
				line = line.lstrip('0123456789.-*• \t')
				if line and len(line) > 3:  # Skip very short lines
					topics.append(line)
		
		return topics
		
	def _update_results(self, topics: List[str], response: str) -> None:
		"""Update UI with generation results."""
		self.progress.stop()
		
		if topics:
			self.result_topics = topics
			self.result_text.delete('1.0', tk.END)
			self.result_text.insert(tk.END, f"Generated {len(topics)} topics:\n\n")
			
			for i, topic in enumerate(topics, 1):
				self.result_text.insert(tk.END, f"{i}. {topic}\n")
			
			self.result_text.insert(tk.END, f"\n--- Full Response ---\n{response}")
		else:
			self.result_text.delete('1.0', tk.END)
			self.result_text.insert(tk.END, f"Failed to generate topics.\n\n{response}")
		
	def use_topics(self) -> None:
		"""Use the generated topics."""
		if self.result_topics:
			self.callback(self.result_topics)
			self.dialog.destroy()
		else:
			messagebox.showwarning("Warning", "No topics generated yet.")
		
	def cancel(self) -> None:
		"""Cancel the dialog."""
		self.dialog.destroy()


if __name__ == "__main__":
	app = ConfigGUI(DEFAULT_CONFIG_PATH)
	app.run()
