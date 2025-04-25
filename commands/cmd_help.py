# commands/cmd_help.py
# Урода, лопающая красивы паттерн команда. Нужно создать абстрактный класс команда и упаковывать аргументы
from commands.base import BaseCommand
from pathlib import Path
from typing import Dict, Type
import argparse
from commands.registry import CommandRegistry

class HelpCommand(BaseCommand):
    def __init__(self, project_root: Path, registry: CommandRegistry):
        self.project_root = project_root
        self.registry = registry  # Получаем доступ к реестру команд

    @property
    def name(self) -> str:
        return "show_help"
    
    @property
    def description(self) -> str:
        return "Show help information about commands"

    def setup_parser(self, parser):
        parser.add_argument(
            "command", 
            nargs="?", 
            help="Specific command to show help for"
        )

    def execute(self, args):
        if not self.registry:
            print("Error: Registry not initialized")
            return
            
        commands = self.registry.instantiate_commands()  # Получаем экземпляры команд
        
        if args.command:
            self._show_command_help(args.command, commands)
        else:
            self._show_general_help(commands)

    def _show_general_help(self, commands: Dict[str, BaseCommand]):
        print("Available commands:\n")
        for name, cmd in commands.items():
            print(f"  {name:<15} {cmd.description}")
        print("\nUse 'show_help <command>' for detailed help")

    def _show_command_help(self, command_name: str, commands: Dict[str, BaseCommand]):
        if command_name not in commands:
            print(f"Unknown command: {command_name}")
            return

        command = commands[command_name]
        print(f"\nHelp for command '{command_name}':")
        print(f"Description: {command.description}\n")

        # Используем argparse для вывода help команды
        import io
        from contextlib import redirect_stdout
        
        buffer = io.StringIO()
        with redirect_stdout(buffer):
            # Создаем временный парсер
            parser = argparse.ArgumentParser(
                prog=f"main.py {command_name}",
                formatter_class=argparse.RawTextHelpFormatter,
                add_help=False
            )
            command.setup_parser(parser)
            parser.print_help()
        
        print(buffer.getvalue())

    def _check_preconditions(self):
        return self.state.state == "uninitialized"  