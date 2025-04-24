# project_root/main.py
from interfaces.cli import CLI
from core.system_state import SystemState

if __name__ == "__main__":
    cli = CLI()
    cli.run()