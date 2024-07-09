import configparser
import os
import json

class ConfigManager:
    def __init__(self, filepath='app_config.ini', default_section='UserPreferences'):
        """Инициализация менеджера конфигураций с путём к файлу."""
        self.config = configparser.ConfigParser()
        self.filepath = filepath
        self.current_section = default_section
        # Загрузка конфигурации, если файл существует
        if os.path.exists(filepath):
            self.config.read(filepath)

    def set_section(self, section):
        """Установка текущей секции для работы."""
        self.current_section = section

    def get(self, option, fallback=None):
        """Получение значения из текущей секции конфигурации."""
        # читаем как json с типом данных
        if not self.config.has_option(self.current_section, option):
            return fallback
        json_data = self.config.get(self.current_section, option)
        try:
            # {type: 'int', value: "10"} - записи такого типа
            data = json.loads(json_data)
            if data['type'] == 'int':
                return int(data['value'])
            elif data['type'] == 'float':
                return float(data['value'])
            elif data['type'] == 'bool':
                return data['value'] == 'True'
            elif data['type'] == 'str':
                return data['value']
            elif data['type'] == 'json':
                return json.loads(data['value'])
            else:
                return fallback
        except json.JSONDecodeError:
            return fallback


    def set(self, option, value):
        """Установка значения в текущей секции конфигурации."""
        if self.current_section not in self.config:
            self.config[self.current_section] = {}
        # записываем как json с типом данных
        if type(value) is int:
            self.config[self.current_section][option] = json.dumps({'type': 'int', 'value': str(value)})
        elif type(value) is float:
            self.config[self.current_section][option] = json.dumps({'type': 'float', 'value': str(value)})
        elif type(value) is bool:
            self.config[self.current_section][option] = json.dumps({'type': 'bool', 'value': str(value)})
        elif type(value) is str:
            self.config[self.current_section][option] = json.dumps({'type': 'str', 'value': str(value)})
        else: # дампнем как json
            self.config[self.current_section][option] = json.dumps({'type': 'json', 'value': json.dumps(value)})
    
    def save(self):
        """Сохранение текущей конфигурации в файл."""
        with open(self.filepath, 'w') as configfile:
            self.config.write(configfile)
    
    def remove_section(self):
        """Удаление текущей секции из конфигурации."""
        if self.current_section and self.config.remove_section(self.current_section):
            self.save()

    def remove_option(self, option):
        """Удаление опции из текущей секции."""
        if self.config.remove_option(self.current_section, option):
            self.save()