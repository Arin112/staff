import tkinter as tk

# Декоратор Spoiler
def Spoiler(wrapped_class, parent, configs, HideShowText=["Скрыть", "Показать"], HW=[300, 600]):
    class SpoilerWrapper(tk.Frame):
        def __init__(self, *args, **kwargs):
            super().__init__(parent)
            self.pack_propagate(False)
            self.is_open = True
            self.resize_mode = False
            self.resize_proc = False

        def init_ui(self):
            self.frame = tk.Frame(self)
            # self.frame.pack_propagate(False)
            # self.frame.pack(anchor='ne')
            self.frame.grid(sticky='ne')
            # self.frame.config(border=6, relief=tk.RAISED)

            self.btn = tk.Button(self.frame, text=HideShowText[0], command=self.toggle)
            self.btn.grid(row=0, column=0, sticky='ne', padx=5, pady=5)
            
            self.panel = tk.Frame(self.frame)
            self.panel.grid(row=1, column=0, sticky='ne', padx=5, pady=5)
            
            self.panel.wrapped_instance = wrapped_class(self.panel, configs)
            self.panel.wrapped_instance.pack(fill=tk.BOTH, expand=True)

            # добавим к panel возможность растягивать за левый нижний угол
            self.panel.bind("<Button-1>", self.on_panel_resize_start)
            self.panel.bind("<B1-Motion>", self.on_panel_resize)
            self.panel.bind("<ButtonRelease-1>", self.on_panel_resize_stop)

            # добавим границу за которую можно растягивать
            self.panel.config(border=6, relief=tk.RAISED)

            self.panel.pack_propagate(False)
            self.panel.config(height=HW[0], width=HW[1])

            self.toggle()
        
        def get_subclass_instance(self):
            return self.panel.wrapped_instance

        def on_panel_resize_start(self, event):
            self.resize_mode = True
            self.resize_x = event.x_root
            self.resize_y = event.y_root
            self.old_width = self.panel.winfo_width()
            self.old_height = self.panel.winfo_height()
        
        def on_panel_resize(self, event):
            if self.resize_proc:
                return
            self.resize_proc = True
            if self.resize_mode:
                new_width = self.old_width - event.x_root + self.resize_x
                new_height = self.old_height + event.y_root - self.resize_y

                new_width = max(100, new_width)
                new_height = max(100, new_height)

                new_width = min(1024, new_width)
                new_height = min(1000, new_height)

                self.panel.config(width=new_width, height=new_height)
                self.winfo_toplevel().update()
            self.resize_proc = False
            
        def on_panel_resize_stop(self, event):
            self.resize_mode = False
            self.resize_proc = False
            new_width = self.old_width - event.x_root + self.resize_x
            new_height = self.old_height + event.y_root - self.resize_y

            new_width = max(100, new_width)
            new_height = max(100, new_height)

            new_width = min(1024, new_width)
            new_height = min(1000, new_height)

            self.panel.config(width=new_width, height=new_height)
            self.winfo_toplevel().update()

        def toggle(self):
            self.is_open = not self.is_open
            if self.is_open:
                self.panel.grid(row=1, column=0, sticky='nsew')
                self.btn.config(text=HideShowText[0])
            else:
                self.btn.config(text=HideShowText[1])
                self.panel.grid_forget()

    return SpoilerWrapper()