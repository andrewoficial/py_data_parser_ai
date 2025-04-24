# commands/registry.py
import importlib
import pkgutil
from pathlib import Path
from typing import Dict, Type
from .base import BaseCommand

class CommandRegistry:
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.command_classes = self._discover_commands()
    
    def _discover_commands(self) -> Dict[str, Type[BaseCommand]]:
        commands = {}
        package = __import__('commands', fromlist=[''])
        
        for _, module_name, _ in pkgutil.iter_modules(package.__path__):
            if module_name.startswith('cmd_'):
                module = importlib.import_module(f'commands.{module_name}')
                for attr_name in dir(module):
                    attr = getattr(module, attr_name)
                    if (isinstance(attr, type) and 
                        issubclass(attr, BaseCommand) and 
                        attr != BaseCommand):
                        
                        # Создаем временный экземпляр для получения имени
                        instance = attr(self.project_root)
                        commands[instance.name] = attr
        return commands
    
    def instantiate_commands(self) -> Dict[str, BaseCommand]:
        return {
            name: command_class(self.project_root)
            for name, command_class in self.command_classes.items()
        }