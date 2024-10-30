function loadSourceFile() { 
    fetch('../files/fib_nums.txt')
    .then(response => response.text())
    .then(data => {
        let content = document.getElementById('content');
        content.textContent = data;
    })
  .catch(err => console.error(err));
}