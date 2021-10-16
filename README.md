[![Build status](https://ci.appveyor.com/api/projects/status/38vohc1jc9ol72dp/branch/master?svg=true)](https://ci.appveyor.com/project/reficul0/algorithmpractice/branch/master)<br>
![language](https://img.shields.io/badge/language-c++-blue.svg)
![c++](https://img.shields.io/badge/std-c++14-blue.svg)

## Задача

Удалить наименьшее число элементов из массива, чтобы элементы в нём удовлетворяли указанному условию(были по возрастанию).

### Реализация
Детали:
* [x] Динамическое программирование, никакой эвристики.
* [x] Кроссплатформенность.

## Установка

### Зависимости

* [Обязательно] - [Conan](https://conan.io/).
* [Устанавливается через conan] - [Boost 1.70](http://www.boost.org/).

### Сборка на Windows

```shell
- git clone git@github.com:reficul0/AlgorithmPractice.git
- mkdir cliver.build && cd cliver.build
- conan install ..
- cmake -A %platform% -G "Visual Studio 15 2017" ..
- cmake --build .
```
### Сборка на Linux

```bash
$ git clone git@github.com:reficul0/AlgorithmPractice.git
$ mkdir cliver.build && cd cliver.build
$ conan install ..
$ cmake ..
$ cmake --build .
```
## Совместимость

ОС           | Компилятор    | Статус
------------ | ------------- | -------------
Windows      | msvc15        | :white_check_mark: Работает
Linux        | gcc           | :white_check_mark: Работает

## Автор

* [@reficul0](https://github.com/reficul0)
