# commands/cmd_help.py
from commands.base import BaseCommand
from pathlib import Path

class HelpCommand(BaseCommand):
    def __init__(self, project_root: Path):
        self.project_root = project_root

    @property
    def name(self) -> str:
        return "help"
    
    @property
    def description(self) -> str:
        return "Show help information about commands"

    def setup_parser(self, parser):
        parser.add_argument(
            'command',
            nargs='?',
            help='Command to show help for'
        )

    def execute(self, args):
        from interfaces.cli import CLI  # Импорт здесь чтобы избежать циклических зависимостей
        cli = CLI()
        print("Инициализация...")
        self.state.set_state("ready") 
        if args.command:
            # Показываем help для конкретной команды
            self._show_command_help(args.command, cli)
        else:
            # Общий help
            cli._print_help()

    def _show_command_help(self, command_name: str, cli):
        if command_name in cli.commands:
            print(f"\nHelp for command '{command_name}':")
            print(f"  Description: {cli.commands[command_name].description}")
            
            # Создаем временный парсер для вывода help конкретной команды
            import io
            from contextlib import redirect_stdout
            
            buffer = io.StringIO()
            with redirect_stdout(buffer):
                temp_parser = argparse.ArgumentParser(
                    prog=f"main.py {command_name}",
                    description=cli.commands[command_name].description,
                    formatter_class=argparse.RawTextHelpFormatter
                )
                cli.commands[command_name].setup_parser(temp_parser)
                temp_parser.print_help()
            
            print(buffer.getvalue())
        else:
            print(f"Unknown command: {command_name}")
            print("Available commands:")
            for name in cli.commands:
                print(f"  {name}")

    def _check_preconditions(self):
        return self.state.state == "uninitialized"                    