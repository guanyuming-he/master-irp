import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext
from tkinter import font as tkfont
import requests
import mimetypes
import json
import os
import base64

# The url is from Ollama doc.
OLLAMA_URL = "http://localhost:11434/api/generate"

# I picked these two models.
TEXT_MODEL = "llama3.1:8b"
FILE_MODEL = "qwen2.5vl:7b"

OUTPUT_FILE = "ollama_output.txt"

BACKGROUND_INSTRUCTION = \
"""The prompts and files given to you will always specify kinds of business
related information. Your task is always to generate search engine prompts that
search for these kinds of information. You are also expected to use boolean
connectives like AND, OR to enhance the prompts, but you don't have to use them
on all of the prompts.  You must not output instructions to the users. You must
output the prompts only. Try to make quite some prompts to cover the desired
kinds of information comprehensively.  """

filepaths = []

def send_to_ollama(model: str, prompt: str, images=[]):
    if (prompt == "" or prompt is None) and len(images) == 0:
        raise RuntimeError("both text prompt and attached files are empty")

    payload = {
        "model" : model,
        "system" : BACKGROUND_INSTRUCTION,
        "stream" : False
    }
    if prompt is not None and prompt != "":
        payload["prompt"] = prompt + " search engine prompts"
    else: # images not empty
        payload["prompt"] = "see attached images and generate search engine"
        + "prompts"
    if len(images) != 0:
        payload["images"] = images
    try:
        response = requests.post(OLLAMA_URL, json=payload)
        response.raise_for_status()
        result = response.json()["response"]
        return result
    except requests.RequestException as e:
        return f"Error communicating with Ollama: {e}"

def process_text_and_file_input(text):
    global filepaths
    output_parts = []
    model: str = None
    if len(filepaths) > 0:
        model = FILE_MODEL
    else:
        model = TEXT_MODEL

    encoded = []
    for fp in filepaths:
        try:
            with open(fp, "rb") as f:
                content = f.read()
            encoded.append(base64.b64encode(content).decode("ascii"))
        except Exception as e:
            output_parts.append(f"Failed to read or encode file: {e}")

    output_parts.append(send_to_ollama(
        model, prompt=text, 
        images=encoded
    ))
    combined_output = "\n".join(output_parts)
    write_output(combined_output)

def write_output(text):
    with open(OUTPUT_FILE, "a", encoding="utf-8") as f:
        f.write(text)
    messagebox.showinfo(
        "Response saved", f"Response written to {OUTPUT_FILE}"
    )


def on_file_drop():
    attached = filedialog.askopenfilenames()
    if attached:
        global filepaths
        filepaths += list(attached)
        upd_filenames()

def on_start():
    text = text_input.get("1.0", tk.END).strip()
    process_text_and_file_input(text)

def clear_files():
    global filepaths
    filepaths = []
    upd_filenames()

def upd_filenames():
    attached_filenames.config(state=tk.NORMAL)
    attached_filenames.delete("1.0", tk.END)
    attached_filenames.insert(
        "1.0", ", ".join(os.path.basename(f) for f in filepaths)
    )
    attached_filenames.config(state=tk.DISABLED)


# GUI setup
root = tk.Tk()
root.title("Prototype 1")

# Base font
base_font = tkfont.Font(family="Helvetica", size=22)

frame = tk.Frame(root, padx=10, pady=10)
frame.pack(fill=tk.BOTH, expand=True)

# Textual description.
text_input = scrolledtext.ScrolledText(
    frame, wrap=tk.WORD, height=10,
    font=base_font
)
text_input.pack(fill=tk.BOTH, expand=True, pady=5)

# Display attached file paths.
attached_filenames = tk.Text(frame, height=5, font=base_font)
attached_filenames.pack()
attached_filenames.config(state=tk.DISABLED)

# Attach files
btn_file = tk.Button(frame, text="Attach file(s)", command=on_file_drop,
                     font=base_font)
btn_file.pack(side=tk.LEFT, padx=5, pady=5)

# Start generation
btn_start = tk.Button(frame, text="Start", command=on_start, font=base_font)
btn_start.pack(side=tk.LEFT, padx=5, pady=5)

# Clear all attached files
btn_clear = tk.Button(frame, text="Clear files", command=clear_files,
                      font=base_font)
btn_clear.pack(side=tk.LEFT, padx=5, pady=5)


root.mainloop()

