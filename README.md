# j2qjs
It is a wrapper of QuickJS as a ScriptEngine for Java.
## Features
- Some customizations to call javaScript modules.
- Java ScriptEngine interface.
- Synchronized every wrapper API. (I am so sorry to do it.)

## How to use
- Copy j2qjs.jar to your classpath.
- Copy libj2qjs.dll to where JNI lib can be searched.

If it is tomcat, copy the dll to tomcat/bin.
If it is a batch, set the dll folder path to classpath.

```java
ScriptEngine se=(new ScriptEngineManager()).getEngineByName("QuickJS");
se.eval("std.loadScript('tests/test_language.js');");//load a javascript file.
se.eval("std.loadModule('tests/test_bignum.js');");//load a javascript module file.
JSObject obj=(JSObject)se.eval("var x={a:1,b:2};x;");
System.out.println("obj="+obj.stringify());
obj.freeValue();
JSFunction fnc=(JSFunction)se.eval("var f=function(a){console.log(a);};f;");
fnc.call("i am a function;");
fnc.freeValue();
JSObject obj2=(JSObject)se.eval("var x={a:1,b:2,c:function(x,y){return this.a+this.b+x+y;}};x;");
Invocable ic=(Invocable)se;
int ret=(int)ic.invokeMethod(obj2, "c", 3,4);
obj2.freeValue();
```

## References
[quickjs-wrapper](https://github.com/HarlonWang/quickjs-wrapper)<br>
[quickjs](https://github.com/bellard/quickjs)