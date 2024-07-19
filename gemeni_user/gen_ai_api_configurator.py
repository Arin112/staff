import tkinter as tk
from tkinter import ttk
import google.generativeai as genai
from logger import logger

genai_configured = False

def check_api_key():
    try :
        genai.list_models()
        return True
    except Exception as e:
        return False

def list_models():
    try:
        if not genai_configured:
            raise Exception("GenAI API not configured")
        return [m.name for m in genai.list_models()]
    except Exception as e:
        return []

class GenAIAPIConfigurator(tk.Frame):
    def __init__(self, parent, configs):
        super().__init__(parent)
        self.configs = configs
        self.pack_propagate(False)
        self.config(bd=2, relief=tk.RAISED)
        self.init_ui()

    def init_ui(self):

        self.api_key_label = tk.Label(self, text="API Key:")
        self.api_key_label.grid(row=0, column=0, sticky="w", padx=5, pady=5)
        self.api_key_entry = tk.Entry(self, width=50, show='•')
        self.api_key_entry.grid(row=0, column=1, sticky="ew", padx=5, pady=5)
        self.api_key_entry.bind("<KeyRelease>", self.on_api_key_change)

        self.model_label = tk.Label(self, text="Model:")
        self.model_label.grid(row=1, column=0, sticky="w", padx=5, pady=5)
        self.available_models = list_models()
        self.model_var = tk.StringVar(self)
        self.model_var.set(self.available_models[0] if self.available_models else '')
        self.model_dropdown = ttk.Combobox(self, textvariable=self.model_var, values=self.available_models, state="readonly")
        self.model_dropdown.grid(row=1, column=1, sticky="ew", padx=5, pady=5)
        self.model_dropdown.bind("<<ComboboxSelected>>", self.on_model_change)

        self.temperature_label = tk.Label(self, text="Temperature:")
        self.temperature_label.grid(row=2, column=0, sticky="w", padx=5, pady=5)
        self.temperature_entry = tk.Entry(self, width=10)
        self.temperature_entry.grid(row=2, column=1, sticky="w", padx=5, pady=5)
        self.temperature_entry.insert(0, str(self.configs.get('temperature', fallback='1.0')))
        self.temperature_entry.bind("<KeyRelease>", self.on_temperature_change)

        self.candidate_count_label = tk.Label(self, text="Candidate Count:")
        self.candidate_count_label.grid(row=3, column=0, sticky="w", padx=5, pady=5)
        self.candidate_count_entry = tk.Entry(self, width=10)
        self.candidate_count_entry.grid(row=3, column=1, sticky="w", padx=5, pady=5)
        self.candidate_count_entry.insert(0, str(self.configs.get('candidate_count', fallback='1')))
        self.candidate_count_entry.bind("<KeyRelease>", self.on_candidate_count_change)

        self.top_k_label = tk.Label(self, text="Top K:")
        self.top_k_label.grid(row=4, column=0, sticky="w", padx=5, pady=5)
        self.top_k_entry = tk.Entry(self, width=10)
        self.top_k_entry.grid(row=4, column=1, sticky="w", padx=5, pady=5)
        self.top_k_entry.insert(0, str(self.configs.get('top_k', fallback='40')))
        self.top_k_entry.bind("<KeyRelease>", self.on_top_k_change)

        self.top_p_label = tk.Label(self, text="Top P:")
        self.top_p_label.grid(row=5, column=0, sticky="w", padx=5, pady=5)
        self.top_p_entry = tk.Entry(self, width=10)
        self.top_p_entry.grid(row=5, column=1, sticky="w", padx=5, pady=5)
        self.top_p_entry.insert(0, str(self.configs.get('top_p', fallback='0.95')))
        self.top_p_entry.bind("<KeyRelease>", self.on_top_p_change)

    def get_current_config(self):
        result = {
            'api_key': self.api_key_entry.get(),
            'model': self.model_var.get(),
            'temperature': self.temperature_entry.get(),
            'candidate_count': self.candidate_count_entry.get(),
            'top_k': self.top_k_entry.get(),
            'top_p': self.top_p_entry.get()
        }
        return result
    
    def is_configured(self):
        global genai_configured
        return genai_configured

    def on_api_key_change(self, event=None):

        if len(self.api_key_entry.get()) != 39:
            self.api_key_entry.config(bg='red')
            global genai_configured
            genai_configured = False
            return

        genai.configure(api_key=self.api_key_entry.get())

        if not check_api_key():
            self.api_key_entry.config(bg='red')
            global genai_configured
            genai_configured = False
            logger.log("GenAI API not configured")
        else:
            self.api_key_entry.config(bg='lightgreen')
            global genai_configured
            genai_configured = True
            logger.log("GenAI API configured ok")

        self.after_update_api_key()

    def after_update_api_key(self):
        self.available_models = list_models()
        logger.log(f"Available models: {self.available_models}")
        # filter available_models
        self.available_models = [m for m in self.available_models if m.startswith('gemini-1.5')]
        logger.log(f"Filtered models: {self.available_models}")
        self.model_dropdown['values'] = self.available_models
        self.model_var.set(self.available_models[0] if self.available_models else '')


    def on_model_change(self, event=None):
        self.configs.set('model', self.model_var.get())
        self.configs.save()

    def on_temperature_change(self, event=None):
        try:
            temperature = float(self.temperature_entry.get())
            self.configs.set('temperature', temperature)
            self.configs.save()
        except ValueError:
            pass

    def on_candidate_count_change(self, event=None):
        try:
            candidate_count = int(self.candidate_count_entry.get())
            self.configs.set('candidate_count', candidate_count)
            self.configs.save()
        except ValueError:
            pass

    def on_top_k_change(self, event=None):
        try:
            top_k = int(self.top_k_entry.get())
            self.configs.set('top_k', top_k)
            self.configs.save()
        except ValueError:
            pass

    def on_top_p_change(self, event=None):
        try:
            top_p = float(self.top_p_entry.get())
            self.configs.set('top_p', top_p)
            self.configs.save()
        except ValueError:
            pass