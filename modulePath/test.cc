#include <nan.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

using namespace std;
using namespace Nan;

//Функция получения имени файла - в теории должна быть мультиплатформенно
string GetFileName(const string& s) {
  char sep = '/';

  #ifdef _WIN32
    sep = '\\';
  #endif

  size_t i = s.rfind(sep, s.length());
  if (i != string::npos) {
    return(s.substr(i+1, s.length() - i));
  }

  return("");
}

// Копирование файла происходит этой функцией
int CopyFile (const string input, const string output)
{
    std::ifstream  src(input, std::ios::binary);
    std::ofstream  dst(output, std::ios::binary);

    dst << src.rdbuf();

    return 0;
}

// А так происходит асинхроная магия. 
class ProgressWorker : public AsyncProgressQueueWorker<char> {
 public:
  ProgressWorker(
      Callback *callback, // Этот callback вызывается после завершения потока (Success Callback)
      Callback *progress, // Это callback высылается после копирования (Progress Callback)
      vector<string> arg0, // Это массив передаваемых файлов
      string arg1) // Это путь куда копируем
    : AsyncProgressQueueWorker(callback), progress(progress), arg0(arg0), arg1(arg1)
     {}

  ~ProgressWorker() {
  }

  void Execute (const AsyncProgressQueueWorker::ExecutionProgress& progress) {

//  Это для мультиплатформенность программы (в теории конечно же)
    string separator(1, '/');

    #ifdef _WIN32
      separator = '\\';
    #endif

// Основной цикл копирования
    for (int i = 0; i < arg0.size(); i++) {

      string filename = GetFileName(arg0[i]); // Забираем как назывался файл ранее
      string newFile = arg1 + separator + filename; // Создаем путь для такого же в новой папке

      CopyFile(arg0[i], newFile); // Копируем

      string msg = "Файл: " + arg0[i] + " скопирован."; // Подготавливаем сообщение о завершении копирования 
      char cmsg[msg.size() + 1]; // Создаем char массиф соответстующий предудыщей строке
      strcpy(cmsg, msg.c_str()); // Переводим string в char*

      progress.Send(reinterpret_cast<const char*>(&cmsg), sizeof(cmsg)); // Прокидываем наш callback прогресса
    }
  }

  // Это Progress Callback который мы посылаем
  void HandleProgressCallback(const char *data, size_t count) {
    HandleScope scope;

    v8::Local<v8::Value> argv[] = {
        Nan::New<v8::String>(data).ToLocalChecked()
    };

    progress->Call(1, argv, async_resource);
  }

  // Это Success Callback который вызывается по заверешнию
  void HandleOKCallback () {
    HandleScope scope;

    v8::Local<v8::Value> argv[] = {
        Nan::New<v8::String>("Выполнение программы окончено").ToLocalChecked()
    };

    callback->Call(1, argv, async_resource);
  }

// Переменные
 private:
  Callback *progress;
  vector<string> arg0;
  string arg1;
};

//Входная функция в модуль
NAN_METHOD(DoProgress) {
  v8::Local<v8::Object> arg0 = info[0]->ToObject(Nan::GetCurrentContext()).FromMaybe(v8::Local<v8::Object>()); // Забираем массив из 1 параметра
  v8::Local<v8::String> arg1 = info[1]->ToString(Nan::GetCurrentContext()).FromMaybe(v8::Local<v8::String>()); // Забираем путь из 2 параметра
  Callback *progress = new Callback(To<v8::Function>(info[2]).ToLocalChecked()); // Забираем Progress Callback из 3 параметра
  Callback *callback = new Callback(To<v8::Function>(info[3]).ToLocalChecked()); // Забираем Progress Callback из 4 параметра

  v8::Local<v8::Array> arg0Props = arg0->GetPropertyNames(Nan::GetCurrentContext()).FromMaybe(v8::Local<v8::Array>()); // Тут свойства нашел массива

  vector<string> filesArray; // Инициализируем список файлов
  int length = arg0Props->Length(); // Смотрим длинну массива

// Через цикл перетаскиваем список файлов из забранного массива в список
  for (int i = 0; i < length; i++) {
      v8::String::Utf8Value utfArg0(v8::Isolate::GetCurrent(), arg0->Get(i));
      string strArg0(*utfArg0);

      filesArray.push_back(strArg0);
  }

  v8::String::Utf8Value utfArg1(v8::Isolate::GetCurrent(), arg1); // Превращаем путь в строку
  string strArg1(*utfArg1); // Превращаем путь в строку

// И передаем все в асинхронную функцию
  AsyncQueueWorker(new ProgressWorker(
      callback,
      progress,
      filesArray,
      strArg1));
}

// Инициализация модуля
NAN_MODULE_INIT(Init) {
  Set(target
    , New<v8::String>("copy").ToLocalChecked()
    , GetFunction(New<v8::FunctionTemplate>(DoProgress)).ToLocalChecked());
}

NODE_MODULE(test, Init)