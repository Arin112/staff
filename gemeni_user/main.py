import config_manager
import tkinter as tk
from file_manager import FileManager
from spoiler import Spoiler
from gen_ai_api_configurator import GenAIAPIConfigurator
# from tkinterhtml import HtmlFrame
# from ceframe import CEFrame
from tkinter import scrolledtext
# import markdown as md
from ttkbootstrap import Style
from tkinter import ttk
import google.generativeai as genai
from tkinter.messagebox import showerror, showwarning, showinfo
from logger import logger

class Application(tk.Tk):
    def __init__(self):
        super().__init__()
        self.style = Style(theme='yeti')
        print(self.style.theme_names())
        self.title("Gemeni code analyzer")
        self.geometry("1024x768")
        self.state('zoomed')

    def init_ui(self):

        # Создание и размещение компонентов
        self.frame = tk.Frame(self)
        self.frame.pack(fill=tk.BOTH, expand=True)
        self.frame.grid_columnconfigure(0, weight=1)
        self.frame.grid_columnconfigure(1, weight=1)
        # self.frame.grid_columnconfigure(2, weight=1)
        self.frame.grid_rowconfigure(0, weight=1)
        # self.frame.grid_rowconfigure(1, weight=1)
        # self.frame.grid_rowconfigure(2, weight=1)
        # self.frame.grid_rowconfigure(3, weight=1)
        self.frame.grid_rowconfigure(4, weight=1)

        # Подключение менеджера конфигураций
        self.configs = config_manager.ConfigManager('app_config.ini', 'UserPreferences')

        # по нижней части будет командная панель
        self.frame.command_frame = tk.Frame(self.frame)
        self.frame.command_frame.grid(column=0, row=4, sticky='nsew', columnspan=3)
        # self.command_frame.pack_propagate(False)
        self.frame.command_frame.config(bd=2, relief=tk.RAISED)
        # минимальный размер командной панели
        # self.frame.command_frame.config(height=250)

        # добавим HtmlFrame
        # self.frame.html_frame = HtmlFrame(self.frame, horizontal_scrollbar="auto", vertical_scrollbar="auto")
        # self.frame.html_frame = CEFrame(self.frame)
        self.frame.html_frame = scrolledtext.ScrolledText(self.frame, wrap=tk.WORD)
        self.frame.html_frame.grid(column=0, row=0, sticky='nsew', columnspan=2, rowspan=3)

        # лог в консоль
        self.frame.command_frame.log_btn = tk.Button(self.frame.command_frame, text="Log", command=self.log) # log debug data
        self.frame.command_frame.log_btn.pack(side=tk.TOP, padx=5, pady=5, anchor='ne')

        # Кнопка переключения языка
        self.frame.command_frame.lang_btn = ttk.Checkbutton(self.frame.command_frame, text='Использовать русский язык в ответе', style='Danger.Roundtoggle.Toolbutton', command=self.toggle_lang)
        self.frame.command_frame.lang_btn.pack(side=tk.TOP, padx=5, pady=5, anchor='sw')

        # Добавим кнопки исполнения команд
        self.create_command_buttons()

        self.frame.spoiler_frame = tk.Frame(self.frame)
        # self.spoiler_frame.pack(fill=tk.BOTH, expand=True, side=tk.RIGHT)
        self.frame.spoiler_frame.grid(column=2, row=0, sticky='ne', rowspan=2)
        # Отобразим границы для отладки
        # self.frame.spoiler_frame.config(border=6, relief=tk.RAISED)

        # Добавим окно для ввода текста
        self.frame.command_frame.text = scrolledtext.ScrolledText(self.frame.command_frame, wrap=tk.WORD)
        self.frame.command_frame.text.pack(fill=tk.BOTH, expand=True, side=tk.BOTTOM)

        self.frame.file_manager = Spoiler(FileManager, self.frame.spoiler_frame, self.configs, ["Свернуть файловый менеджер", "Развернуть файловый менеджер"], [400, 600])
        self.frame.gen_ai_configurator = Spoiler(GenAIAPIConfigurator, self.frame.spoiler_frame, self.configs, ["Скрыть настройки API", "Показать настройки API"], [220, 450])
        # to top right corner
        # self.frame.file_manager.pack(side=tk.TOP, fill=tk.BOTH, expand=True, anchor='nw')
        # self.frame.gen_ai_configurator.pack(side=tk.TOP, fill=tk.BOTH, expand=True, anchor='nw')
        self.frame.file_manager.grid(row=0, column=0, sticky='ne')
        self.frame.gen_ai_configurator.grid(row=1, column=0, sticky='ne')


        # отобразим границы для отладки
        # self.frame.file_manager.config(border=6, relief=tk.RAISED)
        # self.frame.gen_ai_configurator.config(border=6, relief=tk.RAISED)

        self.frame.file_manager.init_ui()
        self.frame.gen_ai_configurator.init_ui()

    def create_command_buttons(self):
        
        # Добавим кнопки
        self.frame.command_frame.btn = tk.Button(self.frame.command_frame, text="Send raw", command=self.send_raw)
        self.frame.command_frame.btn.pack(side=tk.TOP, padx=5, pady=5, anchor='sw')
    
    def toggle_lang(self, event=None):
        # если переключили в русский, то выдать варнинг
        if self.frame.command_frame.lang_btn.instate(['selected']):
            showwarning("Внимание", "Ответы на русском языке существенно дороже из-за особенностей токенизации!")
    
    def send_raw(self):
        # current_config = self.frame.gen_ai_configurator.get_subclass_instance().get_current_config()
        system_instructions = self.get_system_instructions()
        prompt_postfix = self.get_prompt_postfix()
        prompt = self.frame.command_frame.text.get('1.0', tk.END) + prompt_postfix
        
        model = genai.GenerativeModel('gemini-1.5-flash', system_instruction=system_instructions)

        response = model.generate_content(prompt)



        for chunk in response:
            self.frame.html_frame.insert(tk.END, chunk.text)

    def get_prompt_postfix(self):
        postfix = "\n\n"
        if self.frame.command_frame.lang_btn.instate(['selected']):
            postfix += "ОТВЕЧАЙ НА РУССКОМ ЯЗЫКЕ"
        else:
            postfix += "ANSWER IN ENGLISH"
        return postfix

    def get_system_instructions(self):
        system_instructions = "ALWAYS FOLLOW THE INSTRUCTIONS:\n"
        system_instructions += "YOU ARE HIGH-PROFESSIONAL CODE ANALYZER AND PROGRAMMER\n"
        system_instructions += "YOUR MAIN PURPOSE IS TO ANALYZE OR WRITE CODE, GENERATE DOCUMENTATION OR ANSWER QESTIONS ABOUT CODE\n"
        system_instructions += "DO NOT ANSWER ANY QUESTIONS NOT RELATED TO PROGRAMMING, DOCUMENTATION AND OTHER WORK SITUATIONS\n"
        system_instructions += "WHENEVER YOU ARE ASKED TO ANSWER A QUESTION UNRELATED TO YOUR PURPOSE, ANSWER THAT IT IS NOT PART OF YOUR JOB\n"
        system_instructions += "ALWAYS ANSWER IN PROFESSIONAL MANNER\n"
        system_instructions += "ALWAYS ANSWER AS FULLY AS POSSIBLE\n"
        if self.frame.command_frame.lang_btn.instate(['selected']):
            system_instructions += "ВСЕГДА ОТВЕЧАЙ НА РУССКОМ ЯЗЫКЕ НЕЗАВИСИМО ОТ ТОГО НА КАКОМ ЯЗЫКЕ ТЕБЕ ЗАДАЛИ ВОПРОС\n"
        else:
            system_instructions += "ALWAYS ANSWER IN ENGLISH REGARDLESS OF WHAT LANGUAGE YOU WERE ADDRESSED IN\n"
        return system_instructions

    def log(self):

        # для дебага закинем данные ScrolledText в html
        # markdown markup

        # self.frame.html_frame.load_html(md.markdown(self.frame.command_frame.text.get('1.0', tk.END)))
        # self.frame.html_frame.insert(tk.END, self.frame.command_frame.text.get('1.0', tk.END))

        # print("Log data:")
        # print(self.frame.gen_ai_configurator.get_subclass_instance().get_current_config())
        # print(self.frame.file_manager.get_subclass_instance().get_selected_files())

        logger.log("Log data:")
        logger.log(self.frame.gen_ai_configurator.get_subclass_instance().get_current_config())
        logger.log(self.frame.file_manager.get_subclass_instance().get_selected_files())

if __name__ == "__main__":

    # style = Style(theme='yeti')
    # style.theme_use('yeti')

    app = Application()
    app.init_ui()
    app.mainloop()