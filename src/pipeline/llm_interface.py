"""
@author Guanyuming He
Copyright (C) Guanyuming He 2025
The file is licensed under the GNU GPL v3.

LLM Interface.

This files defines the interfaces used to communicate with the LLMs
used in my project.
"""

import requests
import mimetypes
import json
import os
import base64

# The url is from Ollama doc.
OLLAMA_URL = "http://localhost:11434/api/generate"

def send_to_ollama(
	model: str, 
	system: str,
	prompt: str, 
	images: list = [],
	extra_args: dict = {}
):
	"""
	Send text or images or both to ollama.

	@param model the model to use
	@param the system instruction.
	@param prompt the text input
	@param images the images, encoded in base64.

	@throws RuntimeError if no input
	@throws RuntimeError if cannot communicate with Ollama
	"""
	if (prompt == "" or prompt is None) and len(images) == 0:
		raise RuntimeError(
			"both text prompt and attached files are empty"
		)

	payload = {
		"model" : model,
		"system" : system,
		"stream" : False,  
		# Do not remember previous session.
		# "keep_alive": 0
	}
	if prompt is not None and prompt != "":
		payload["prompt"] = prompt
	else: # images not empty
		payload["prompt"] = "see also attached images"
	if len(images) != 0:
		payload["images"] = images
		# restrict the maximum number of tokens, because sometimes,
		# when there is image input, the generation will suddenly give
		# thousands lines of output.
		# Don't use it. May not work. It might also fuck the LLM up.
		# payload["num_predict"] = 32

	for k,v in extra_args:
		if (k in payload):
			raise ValueError(
				f"key {k} in extra_arg is already used."
			)
		payload[k] = v

	try:
		response = requests.post(OLLAMA_URL, json=payload)
		response.raise_for_status()
		result = response.json()["response"]
		return result
	except requests.RequestException as e:
		raise RuntimeError(
			f"Error communicating with Ollama: {e}"
		)
