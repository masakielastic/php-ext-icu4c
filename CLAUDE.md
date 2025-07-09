# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a PHP extension project for ICU4C (International Components for Unicode for C/C++). The project appears to be in its initial setup phase with minimal structure.

## Development Setup

The project is currently in early development with no build system, package management, or testing framework configured yet. The codebase consists of only configuration files.

## Commit Guidelines

This project follows [Conventional Commits](https://www.conventionalcommits.org/) specification for commit messages. Use the following format:

```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

Common types:
- `feat`: A new feature
- `fix`: A bug fix
- `docs`: Documentation only changes
- `style`: Changes that do not affect the meaning of the code
- `refactor`: A code change that neither fixes a bug nor adds a feature
- `test`: Adding missing tests or correcting existing tests
- `chore`: Changes to the build process or auxiliary tools

Examples:
- `feat: add unicode normalization support`
- `fix: resolve memory leak in string conversion`
- `docs: update installation instructions`
- `chore: update build configuration`

## Architecture Notes

- This is a PHP extension project that will interface with the ICU4C library
- The project structure is minimal and will need to be established as development progresses
- No source code files are present yet - this appears to be a fresh project initialization

## Development Commands

No build system or development commands have been configured yet. Standard PHP extension development will likely require:
- `phpize` for preparing the build environment
- `./configure` for configuration
- `make` for building
- `make install` for installation

These commands will need to be verified and documented as the project structure develops.