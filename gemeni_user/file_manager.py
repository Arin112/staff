import tkinter as tk
from ttkwidgets import CheckboxTreeview
import os

class FileManager(tk.Frame):
    def __init__(self, parent, configs):
        super().__init__(parent)
        self.configs = configs
        # self.pack_propagate(False)
        
        # Инициализация основных компонентов
        self.tree_module = TreeModule(self, self.configs)
        self.toolbar = Toolbar(self, self.configs, self.tree_module)

        # Размещение компонентов
        self.toolbar.pack(side=tk.TOP, fill=tk.X)
        self.tree_module.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        # Добавление панели управления
        self.control_panel = ControlPanel(self, self.configs, self.tree_module, width=300)
        self.control_panel.pack(side=tk.RIGHT, fill=tk.Y)

        # Загрузка последней открытой папки
        last_folder = self.configs.get('last_folder')
        if last_folder:
            self.tree_module.set_root(last_folder)
    
    def get_selected_files(self):
        list_files = []
        for item in self.tree_module.tree.get_checked():
            file = self.tree_module.tree.item(item)
            if file['text'] == 'Нет доступа':
                continue
            # получим полный путь к файлу
            path = file['text']
            it = item
            while self.tree_module.tree.parent(it) != '':  # пока не дойдем до корня
                it = self.tree_module.tree.parent(it)
                path = self.tree_module.tree.item(it)['text'] + "/" + path
            list_files.append(path)
        return list_files
    
class TreeModule(tk.Frame):
    def __init__(self, parent, configs):
        super().__init__(parent)
        self.configs = configs
        self.tree = CheckboxTreeview(self)
        scroll = tk.Scrollbar(self, orient="vertical", command=self.tree.yview)
        self.tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scroll.pack(side=tk.RIGHT, fill=tk.Y)
        self.tree.configure(yscrollcommand=scroll.set)

        self.extensions = ['.c', '.cpp', '.h', '.hpp']
        self.use_extensions = self.configs.get('use_extensions', fallback=False)

        self.only_non_empty_folders = self.configs.get('only_non_empty_folders', fallback=False)
    
    def set_root(self, path):
        self.tree.delete(*self.tree.get_children())
        oid = self.tree.insert('', 'end', text=path, open=True)
        self.fill_tree(self.tree, path, oid)
    
    def redraw(self):
        self.set_root(self.tree.item(self.tree.get_children()[0])['text'])
    
    def fill_tree(self, tree, path, node):
        try:
            entries = os.listdir(path)
            # сортируем по имени, сначала папки, потом файлы
            entries.sort(key=lambda x: (not os.path.isdir(os.path.join(path, x)), x))
        except PermissionError:
            print(f"Нет доступа к {path}")
            # добавим подсказку к папке, что нет доступа
            tree.insert(node, 'end', text="Нет доступа", open=False)
            return

        for entry in entries:
            entry_path = os.path.join(path, entry)
            if os.path.isdir(entry_path):
                if self.only_non_empty_folders:
                    try:
                        if not os.listdir(entry_path):
                            continue
                        is_empty = True
                        # рекурсивно проверим все вложенные папки на наличие файлов
                        for root, dirs, files in os.walk(entry_path):
                            for file in files:
                                if self.use_extensions:
                                    if os.path.splitext(file)[1] in self.extensions:
                                        is_empty = False
                                        break
                                else:
                                    is_empty = False
                                    break
                        if is_empty:
                            continue
                    except PermissionError:
                        continue
                oid = tree.insert(node, 'end', text=entry, open=False)
                self.fill_tree(tree, entry_path, oid)
            else:
                if self.use_extensions:
                    if os.path.splitext(entry)[1] not in self.extensions:
                        continue
                    else :
                        tree.insert(node, 'end', text=entry, open=False)
                else:
                    tree.insert(node, 'end', text=entry, open=False)
class ControlPanel(tk.Frame):
    def __init__(self, parent, configs, tree_module, width):
        super().__init__(parent, width=width)
        self.configs = configs
        self.pack_propagate(False)  # Отключаем пропагацию размеров от дочерних элементов
        self.config(bd=2, relief=tk.RAISED)
        self.tree_module = tree_module
        self.init_ui()

    def init_ui(self):
        self.add_check_only_sources()
        self.add_only_non_empty_folders()
    
    def add_check_only_sources(self):
        # Добавление кнопок и других элементов управления
        check_only_sources = tk.Checkbutton(self, text="Только файлы исходного кода [c, cpp, h, hpp]", command=self.on_check_only_sources)
        check_only_sources.pack(padx=5, pady=5)
        check_only_sources.pack(anchor=tk.W)

        # Проверим в конфиге
        self.tree_module.use_extensions = self.configs.get('use_extensions', fallback=False)
        check_only_sources.select() if self.tree_module.use_extensions else check_only_sources.deselect()

    def on_check_only_sources(self):
        self.tree_module.use_extensions = not self.tree_module.use_extensions
        self.tree_module.redraw()
        self.configs.set('use_extensions', self.tree_module.use_extensions)
        self.configs.save()

    def add_only_non_empty_folders(self):
        check_only_non_empty_folders = tk.Checkbutton(self, text="Только непустые папки", command=self.on_check_only_non_empty_folders)
        check_only_non_empty_folders.pack(padx=5, pady=5)
        check_only_non_empty_folders.pack(anchor=tk.W)

        # Проверим в конфиге
        self.tree_module.only_non_empty_folders = self.configs.get('only_non_empty_folders', fallback=False)
        check_only_non_empty_folders.select() if self.tree_module.only_non_empty_folders else check_only_non_empty_folders.deselect()

    def on_check_only_non_empty_folders(self):
        self.tree_module.only_non_empty_folders = not self.tree_module.only_non_empty_folders
        self.tree_module.redraw()
        self.configs.set('only_non_empty_folders', self.tree_module.only_non_empty_folders)
        self.configs.save()



class Toolbar(tk.Frame):
    def __init__(self, parent, configs, tree_module):
        super().__init__(parent, bg='grey')
        self.configs = configs
        self.tree_module = tree_module
        self.init_ui()

    def init_ui(self):
        # Создание кнопок
        # первая кнопка - выбор папки
        self.btn_open = tk.Button(self, text="Выбрать папку", command=self.on_open)
        self.btn_open.pack(side=tk.LEFT, padx=5, pady=5)
    
    def on_open(self):
        path = tk.filedialog.askdirectory()
        if path:
            self.tree_module.set_root(path)
            self.configs.set('last_folder', path)
            self.configs.save()