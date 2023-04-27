# Graph

## A programming language compiler for speed and simplicity
Goal of this language project is to make a low level language, that compiles very quickly and runs quicker than an interpreter.
Eventually we will also hopefully have metaprogramming utilities with compile time exectution similar to Jai.
The compiler is in very early stage development and is missing many basic features.


## Examples
### hello world
```c++
main :: (){
  printf("hello world");
  <- 0;
}
```

### pow function
```c++
pow :: (number: int, to_power: int){
  if(to_power == 0){
    <- 1;
  }
  
  result: int;
  result = number;
  
  counter: int;
  counter = 1;
  
  while(counter < to_power){
    result = result * number;
    counter = counter + 1;
  }
  
  <- result;
}
```
