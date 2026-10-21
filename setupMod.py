import tomllib
import re
from pathlib import Path

path = Path("xmake.lua")
workflow_path = Path(".github/workflows/build.yml")
config_path = Path("moddata.toml")
with open(config_path, "rb") as file:
    config = tomllib.load(file)

def change_modname(text, config):
    return text.replace(
        'local mod_name = "MOD"',
        f'local mod_name = "{config["name"]}"'
    )


def change_license(text, config):
    return text.replace(
        'set_license("MIT")',
        f'set_license("{config["license"]}")'
    )

def change_version(text, config):
    return re.sub(r'set_version\("[^"]*"\)',
    f'set_version("{config["modversion"]}")', 
    text, 
    count=1
    )

def xmake_value(value):
    if isinstance(value, bool):
        return str(value).lower()
    if isinstance(value, str):
        return f'"{value}"'
    return str(value)


def add_configs(text, config):
    configs = config.get("configs", {})

    lines = [
        f'set_config("{name}", {xmake_value(value)})'
        for name, value in configs.items()
    ]

    return text.replace(
        "--{{ADDITIONAL CONFIGS}}--",
        "\n".join(lines) + ("\n" if lines else "")
    )

def change_workflowname(wf, config):
    return wf.replace('MOD_NAME: MOD', f'MOD_NAME: {config["workflowname"]}')

def change_xmakefile(text, config):
    text = change_modname(text, config)
    text = change_license(text, config)
    text = change_version(text, config)
    text = add_configs(text, config)    
    return text

def change_workflow(wf, config):
    wf = change_workflowname(wf, config)
    return wf

def main():
    text = path.read_text(encoding="utf-8")
    wf = workflow_path.read_text(encoding="utf-8")

    text = change_xmakefile(text, config)
    wf = change_workflow(wf, config)

    path.write_text(text, encoding="utf-8")
    workflow_path.write_text(wf, encoding="utf-8")


if __name__ == "__main__":
    main()
