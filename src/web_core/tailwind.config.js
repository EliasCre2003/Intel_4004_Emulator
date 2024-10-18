/** @type {import('tailwindcss').Config} */
module.exports = {
  mode: 'jit',
  content: ["./build/{*.html, html/*.html, js/*.js}"],
  theme: {
    extend: {},
  },
  plugins: [],
}

