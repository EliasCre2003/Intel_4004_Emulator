from browser import document, window

from assembler import assemble_from_str
# window.something()

# module = window.eval("import('./main.js')")
# module.then(lambda m: assemble_file("files/fib_nums.txt", m))

def load_file(file_path: str) -> str:
        # Use JavaScript's fetch API to get the file
                
        def on_success(response):
            return response.text()  # Get the file content as text

        # def on_result(file_content):
        #     global content
        #     # print(file_content)
        #     content = file_content

        def assemble(file_content: str):
            print(file_content)
            code = assemble_from_str(file_content)
            print(code)
            window.initEmulator(code)
        # Fetch the file from the server (assuming it's at the same location)
        window.fetch(file_path).then(on_success).then(assemble)

        # window.setTimeout(lambda: assemble(content), 1000)



def assemble_file(file_path: str) -> str:
    file_content = load_file(file_path)
    print(f"Något jälva helvete {file_content}")
    code = assemble_from_str(file_content)
    window.initEmulator(code)
    

window.setTimeout(lambda: load_file("files/fib_nums.txt"), 1000)
