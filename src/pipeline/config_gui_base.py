"""
@author Guanyuming He
Copyright (C) Guanyuming He 2025
The file is licensed under the GNU GPL v3.

The file defines the base classes and common variables for config_gui.py.
"""

import tkinter as tk
from tkinter import ttk
from typing import Any

from config import Config


# Global font settings
# I put them here instead of inside ConfigGUI because 
# for some elements I have to set their font individually; I cannot do that
# centrally.
DEFAULT_FONT_SIZE = 14
FONT_FAMILY = "Segoe UI"
DEFAULT_FONT = (FONT_FAMILY, DEFAULT_FONT_SIZE)


class ConfigDialog:
	"""
	Base class for all config dialogs that pops up for editing a specific 
	part of the config.

	Roughly speaking, I set up the common GUI elements, in particular a 
	scrollbar, so that the Dialog can contain a lot of elements.
	"""
	def __init__(
		self, 
		parent: tk.Widget,
		w: int, h: int, 
		title: str
	):
		"""
		@param parent of the dialog
		@param w width
		@param h height
		@param title for the dialog
		"""
		self.parent = parent

		self.w = w; self.h = h
		# Many places of tkinter have this special string used to represent
		# size.
		self.wxh_str : str = f"{w}x{h}"
		self.title = title

		# Create the dialog
		self.dialog = tk.Toplevel(self.parent)
		self.dialog.title(self.title)
		self.dialog.geometry(self.wxh_str)
		self.dialog.resizable(True, True)
		self.dialog.transient(self.parent)
		self.dialog.grab_set()

		# Center the dialog
		self.dialog.update_idletasks()
		x = (self.dialog.winfo_screenwidth() // 2) - (self.w // 2)
		y = (self.dialog.winfo_screenheight() // 2) - (self.h // 2)
		self.dialog.geometry(f"{self.wxh_str}+{x}+{y}")

		# Create the widgets
		self.create_widgets()
		self.dialog.wait_window()

	def create_widgets(self) -> None:
		"""
		Create the basic Scrollbar and calls a virtual method
		create_custom_widgets(), where subclasses should use self.inner_frame
		as their parent, not self.dialog.
		"""
		# Make contents scrollable	
		self.container = ttk.Frame(self.dialog)
		self.container.pack(fill="both", expand=True)
		self.canvas = tk.Canvas(self.container)
		self.canvas.pack(side="left", fill="both", expand=True)
		self.scrollbar = ttk.Scrollbar(
			self.container, orient="vertical",
			command=self.canvas.yview
		)
		self.scrollbar.pack(side="right", fill="y")
		self.canvas.configure(yscrollcommand=self.scrollbar.set)
		self.canvas.bind(
			'<Configure>', 
			lambda e: self.canvas.configure(
				scrollregion=self.canvas.bbox("all")
			)
		)
		self.inner_frame = ttk.Frame(self.canvas)
		self.canvas.create_window(
			(0, 0), window=self.inner_frame, anchor="nw"
		)

		# Call the virtual function to enable subclasses of doing their own
		# things.
		self.create_custom_widgets()

	def create_custom_widgets(self) -> None:
		raise NotImplementedError("Abstract method.")



class ConfigSectionFrame:
	"""
	Base class for all config section frames.
	Each config section should inherit from this and implement
	the abstract methods to create GUI elements and handle data.
	"""
	
	def __init__(self, parent: tk.Widget, title: str):
		self.parent = parent
		self.title = title
		self.frame = ttk.LabelFrame(parent, text=title, padding="10")
		self.widgets = {}
		
	def create_widgets(self) -> None:
		"""
		Create the widgets. Nothing interesting here.
		"""
		raise NotImplementedError("Abstract method")
		
	def load_data(self, data: Any) -> None:
		"""
		Load data from Config and display them in the widget.
		"""
		raise NotImplementedError("Abstract method")
		
	def get_data(self) -> Any:
		"""
		Extract data from widgets and return a corresponding object,
		probably a ConfigSection.
		"""
		raise NotImplementedError("Abstract method")
		
	def pack(self, **kwargs) -> None:
		"""
		Just a forwarder of args into self.frame.pack().
		"""
		self.frame.pack(**kwargs)

