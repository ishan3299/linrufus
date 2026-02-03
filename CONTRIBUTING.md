# Contributing to LinRufus

Thank you for your interest in contributing to LinRufus! We welcome contributions from everyone.

## Getting Started

1.  **Fork the repository** on GitHub.
2.  **Clone your fork** locally:
    ```bash
    git clone https://github.com/YOUR_USERNAME/linrufus.git
    ```
3.  **Create a branch** for your feature or bugfix:
    ```bash
    git checkout -b feature/my-new-feature
    ```

## Development Workflow

-   **Build**: Use `make` to build the project.
-   **Test**: Run the tool locally using `sudo ./linrufus --list` or by writing to a test USB drive.
-   **Style**: Please stick to the existing coding style (C11 standard).

## Submitting Changes

1.  **Commit your changes** with clear messages.
2.  **Push to your fork**:
    ```bash
    git push origin feature/my-new-feature
    ```
3.  **Open a Pull Request** against the `master` branch of the main repository.

## Reporting Issues

If you find a bug or have a feature request, please search existing issues first. If no existing issue matches yours, please create a new one using the provided templates.

## Windows ISO Support

If you are working on Windows ISO support, please ensure you test with valid Windows ISOs and `wimlib` installed.

## License

By contributing, you agree that your contributions will be licensed under the GPLv3.
