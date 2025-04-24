# commands/base.py
from abc import ABC, abstractmethod
from core.system_state import SystemState
import argparse

class BaseCommand(ABC):
    def __init__(self, state: SystemState):
        self.state = state

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

    @abstractmethod
    def _check_preconditions(self) -> bool:
        return True  # Переопределять в подклассах