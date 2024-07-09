import tkinter as tk
from cefpython3 import cefpython as cef

class CEFrame(tk.Frame):
    def __init__(self, parent, **kwargs):
        super().__init__(parent, **kwargs)
        self.browser = None
        self.pack(fill=tk.BOTH, expand=True)
        self.parent = parent
        self.init_cef()

    def init_cef(self):
        # Настройка CEF для использования в системе Tkinter
        window_info = cef.WindowInfo(self.get_handle())
        window_info.SetAsChild(self.winfo_id())
        self.browser = cef.CreateBrowserSync(window_info, url="about:blank")

        # Настройка обработчиков сообщений для интеграции CEF в главный цикл Tkinter
        self.parent.after(10, self.update_cef)

    def update_cef(self):
        cef.MessageLoopWork()
        self.parent.after(10, self.update_cef)

    def load_html(self, html_content):
        # Загрузка HTML содержимого в браузер
        self.browser.LoadString(html_content, "about:blank")

    def get_handle(self):
        # Получение дескриптора окна для встраивания CEF
        return self.winfo_id()

    def on_destroy(self):
        # Очистка ресурсов CEF перед закрытием приложения
        self.browser.CloseBrowser(True)
        cef.Shutdown()

if __name__ == "__main__":
    root = tk.Tk()
    root.geometry("800x600")
    app = CEFrame(root)
    app.load_html("""
        <html>
        <body>
        <h1>Hello CEF in Tkinter!</h1>
        <p>This is HTML content inside a Tkinter application.</p>
        </body>
        </html>
    """)
    root.protocol("WM_DELETE_WINDOW", app.on_destroy)
    root.mainloop()
