# commands/base.py
from abc import ABC, abstractmethod
import argparse

class BaseCommand(ABC):
    @property
    @abstractmethod
    def name(self) -> str:
        """Имя команды для вызова из CLI"""
        pass
    
    @property
    @abstractmethod
    def description(self) -> str:
        """Краткое описание команды"""
        pass

    @abstractmethod
    def setup_parser(self, parser: argparse.ArgumentParser):
        """Настройка парсера аргументов"""
        pass
    
    @abstractmethod
    def execute(self, args):
        """Выполнение команды"""
        pass