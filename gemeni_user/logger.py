import datetime
import multiprocessing
import os
import traceback
from typing import Any


class _Logger:
    """
    Логгер, записывающий сообщения в файл с уникальным именем,
    основанным на времени запуска, с использованием multiprocessing.
    Позволяет использовать один и тот же файл журнала во всех сабмодулях.
    """

    # Классовая переменная для хранения единственного экземпляра логгера
    _instance = None

    def __new__(cls, *args, **kwargs):
        """Реализует паттерн Singleton для создания только одного экземпляра класса."""
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self) -> None:
        """Инициализирует логгер, если он еще не был инициализирован."""
        if not hasattr(self, 'initialized'):  # Проверяем, был ли логгер уже инициализирован
            self.initialized = True
            self.copy_functors = []
            self.log_file_path = self._generate_log_file_path()
            self.queue = multiprocessing.Queue()
            self.process = multiprocessing.Process(target=self._process_queue)
            self.process.daemon = True
            self.process.start()

    def _generate_log_file_path(self) -> str:
        """Генерирует уникальное имя файла журнала на основе времени запуска."""
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        return f"log_{timestamp}.txt"

    def _process_queue(self) -> None:
        """Обрабатывает очередь сообщений журнала, записывая их в файл."""
        while True:
            try:
                message = self.queue.get()
                with open(self.log_file_path, "a", encoding="utf-8") as log_file:
                    log_file.write(message + "\n")
            except Exception as e:
                # Логируем ошибку внутри логгера
                print(f"Ошибка записи в лог: {e}")
                traceback.print_exc()

    def add_copy_functor(self, functor: Any) -> None:
        """
        Добавляет функцию-копировщик сообщений журнала.

        Args:
            functor: Функция, которая будет вызываться при каждом добавлении сообщения в журнал.
        """
        self.copy_functors.append(functor)

    def log(self, message: Any) -> None:
        """
        Помещает сообщение в очередь для записи в файл журнала.

        Args:
            message: Сообщение для записи в журнал.
        """
        formatted_message = (
            f"{datetime.datetime.now().isoformat()}: {message}"
        )
        self.queue.put(formatted_message)
        for f in self.copy_functors:
            f(formatted_message)

logger = _Logger()

if __name__ == "__main__":
    logger.log("Пример сообщения в журнале")
    logger.log("Еще одно сообщение в журнале")
    logger.log("И еще одно сообщение в журнале")

    os.system("pause")
