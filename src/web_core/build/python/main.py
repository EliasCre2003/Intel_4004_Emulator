from browser import document, window
from assembler import assemble_from_str


def load_file(file_path: str) -> str:  
        window.fetch(file_path).then(
             lambda response: response.text()
            ).then(
             lambda file_content: window.initEmulator(
                     assemble_from_str(file_content)
                )
            )


def main():
    window.setTimeout(lambda: load_file("files/fib_nums.txt"), 1000)


if __name__ == "__main__":
    main()