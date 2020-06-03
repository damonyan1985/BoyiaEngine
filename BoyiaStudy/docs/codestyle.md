# Code Style

## C++ 代码规范
均采用小驼峰

### 成员变量，m_开头
```
View* m_view
```

### 静态成员
```
Instance* s_instance
```

### 成员函数
```
public:
  void functionTest();
```

### 函数内静态变量
```
Context* CTest::instance()
{
    static Context sContext;
    return &sContext;
}
```

## C 风格

虚拟机部分虽然使用cpp，但属于C规范编程，为方便以后移植到C编译器

### struct成员, m开头
```
struct View {
  LInt mViewId;
};
```

### 函数采用大驼峰
```
void FunctionTest();
```

### 静态变量
```
static Context sContext;
```