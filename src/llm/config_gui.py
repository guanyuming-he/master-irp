"""
@author Guanyuming He
Copyright (C) Guanyuming He 2025
The file is licensed under the GNU GPL v3.

The file creates a GUI that helps the user to write to 
config.json.

Because creating GUI elements is tedious and seldom a work that can improve
me a lot, I let ChatGPT write most functions of the class.
"""

import json
import tkinter as tk
from tkinter import messagebox, simpledialog, filedialog


class ConfigGUI:
	def __init__(self, root):
		self.root = root
		root.title("Config.json Generator")

		self.schedules = {}
		self.email_addresses = []

		self.build_widgets()

	def build_widgets(self):
		"""
		Boring building widgets.
		Nothing interesting here.
		"""
		tk.Label(self.root, text="Sender Email:").grid(row=0, column=0)
		self.sender_email_entry = tk.Entry(self.root, width=40)
		self.sender_email_entry.grid(row=0, column=1)

		tk.Label(self.root, text="Sender Password:").grid(row=1, column=0)
		self.sender_password_entry = tk.Entry(
			self.root, show='*', width=40)
		self.sender_password_entry.grid(row=1, column=1)

		self.add_schedule_btn = tk.Button(
			self.root, text="Add Schedule", command=self.add_schedule)
		self.add_schedule_btn.grid(row=2, column=0, pady=10)

		self.add_email_btn = tk.Button(
			self.root, text="Add Email", command=self.add_email)
		self.add_email_btn.grid(row=2, column=1)

		self.save_btn = tk.Button(
			self.root, text="Save Config", command=self.save_config)
		self.save_btn.grid(row=3, column=0, columnspan=2, pady=20)

	def add_schedule(self):
		"""
		format in json:
		<name>: {
			"schedule": ??,
			"every_x_days": ??,
			"time": ??,
			"command": ??,
			"catch_up": ??
		},
		"""
		name = simpledialog.askstring("Job Name", "Enter schedule name:")
		if not name:
			return

		tk.Label(win, text="Schedule Type:").grid(row=1, column=0)
		sched_var = StringVar(win)
		sched_var.set("every_x_days")
		sched_menu = tk.OptionMenu(
			win, sched_var, "every_x_days", "weekly")
		sched_menu.grid(row=1, column=1)

		time = simpledialog.askstring("Time", "Enter time (HH:MM):")
		cmd = simpledialog.askstring("Command", "Enter command to run:")
		catch_up = messagebox.askyesno("Catch Up?",
			"Should this job catch up if missed?")

		job = {
			"schedule": sched_type,
			"time": time,
			"command": cmd,
			"catch_up": catch_up
		}

		if sched_type == "every_x_days":
			every = simpledialog.askinteger("Every X Days",
				"Enter number of days:")
			job["every_x_days"] = every
		elif sched_type == "weekly":
			day = simpledialog.askinteger("Day",
				"Enter weekday (0=Sun, 6=Sat):")
			job["day"] = day

		self.schedules[name] = job
		messagebox.showinfo("Added", f"Added schedule '{name}'")

	def add_email(self):
		email = simpledialog.askstring("Email", "Enter email address:")
		if email:
			self.email_addresses.append(email)
			messagebox.showinfo("Added", f"Added email '{email}'")

	def save_config(self):
		data = {
			"schedules": self.schedules,
			"email_addresses": self.email_addresses,
			"sender_email": self.sender_email_entry.get(),
			"sender_password": self.sender_password_entry.get()
		}

		file_path = filedialog.asksaveasfilename(
			defaultextension=".json",
			filetypes=[("JSON files", "*.json")])

		if not file_path:
			return

		with open(file_path, "w") as f:
			json.dump(data, f, indent=2)

		messagebox.showinfo("Success", "Configuration saved.")


if __name__ == "__main__":
	root = tk.Tk()
	app = ConfigGUI(root)
	root.mainloop()
