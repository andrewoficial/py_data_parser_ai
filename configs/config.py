from pydantic import BaseSettings
from pathlib import Path


class Settings(BaseSettings):
    db_host: str = "localhost"
    db_user: str = "admin"
    db_password: str = "secret"
    db_name: str = "mydb"

    openrouter_api_key: str = "your-api-key-here"
    vector_db_path: Path = Path("/path/to/vector/db")

    class Config:
        env_file = ".env"
        env_file_encoding = "utf-8"


# Экземпляр настроек
settings = Settings()