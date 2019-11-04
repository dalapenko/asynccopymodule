//const test = require('./modulePath/test');
const test = require('bindings')('test'); // UNIX

//const files = ['C:\\files\\text.txt', 'C:\\files\\image.png']; // Пример массива путей к файлам
const files = ['/Users/dalapenko/files/text1.txt', '/Users/dalapenko/files/image.png']; // Модифицированно для UNIX
//const location = 'C:\\location'; // Пример папки, в которую нужно скопировать файлы
const location = '/Users/dalapenko/location'; // Модифицировано для UNIX


function printProgress(text) {
    console.log(text);
} // Функция callback, выводящая информацию о скопированном файле. Пример text – “Файл C:\\files\\text.txt скопирован”.

function printSuccess(text) {
    console.log(text);
} // Функция callback, выводящая сообщение о завершении копирования


test.copy(files, location, printProgress, printSuccess);