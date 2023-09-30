
import java.io.IOException;
import java.nio.file.Paths;
import java.util.List;

import javax.script.Bindings;
import javax.script.Compilable;
import javax.script.Invocable;
import javax.script.ScriptContext;
import javax.script.ScriptEngine;
import javax.script.ScriptEngineFactory;
import javax.script.ScriptEngineManager;
import javax.script.ScriptException;

import efw.script.j2qjs.engine.J2qjsScriptEngine;
import efw.script.j2qjs.engine.J2qjsScriptEngineFactory;
import efw.script.j2qjs.wrapper.JSArray;
import efw.script.j2qjs.wrapper.JSFunction;
import efw.script.j2qjs.wrapper.JSObject;

public class Test {
	public static ScriptEngine se=(new ScriptEngineManager()).getEngineByName("QuickJS");
	public static void main(String[] args) throws ScriptException, NoSuchMethodException, IOException {
		System.out.println("=================J2qjsScriptEngineFactory==============================");
		J2qjsScriptEngineFactory f=new J2qjsScriptEngineFactory();
		System.out.println("getEngineName="+f.getEngineName());
		System.out.println("getEngineVersion="+f.getEngineVersion());
		System.out.println("getExtensions="+f.getExtensions().toString());
		System.out.println("getMimeTypes="+f.getMimeTypes().toString());
		System.out.println("getNames="+f.getNames().toString());
		System.out.println("getLanguageName="+f.getLanguageName().toString());
		System.out.println("getLanguageVersion="+f.getLanguageVersion().toString());
		System.out.println("getParameter(javax.script.name)="+f.getParameter("javax.script.name"));
		System.out.println("getParameter(javax.script.engine)="+f.getParameter("javax.script.engine"));
		System.out.println("getParameter(javax.script.engine_version)="+f.getParameter("javax.script.engine_version"));
		System.out.println("getParameter(javax.script.language)="+f.getParameter("javax.script.language"));
		System.out.println("getParameter(javax.script.language_version)="+f.getParameter("javax.script.language_version"));
		System.out.println("getMethodCallSyntax(obj,method,args)="+f.getMethodCallSyntax("j2qjs", "sayHello","to","you"));
		System.out.println("getOutputStatement(toDisplay)="+f.getOutputStatement("j2qjs.sayHello()"));
		System.out.println("getProgram(statements)="+f.getProgram("hello()","bye()"));
		System.out.println("getScriptEngine()="+f.getScriptEngine());
		
		System.out.println("=================javax.script.ScriptEngineFactory======================");
		List<ScriptEngineFactory> lst=(new ScriptEngineManager()).getEngineFactories();
		for(int i=0;i<lst.size();i++) {
			ScriptEngineFactory f0=lst.get(i);
			String scrptnms=f0.getNames().toString();
			System.out.println(scrptnms);
		}
		ScriptEngine se=(new ScriptEngineManager()).getEngineByName("QuickJS");
		if (se==null) {
			System.out.println("(new ScriptEngineManager()).getEngineByName(QuickJS) = null");
			return;
		}else {
			System.out.println("(new ScriptEngineManager()).getEngineByName(QuickJS) = "+se);
		}
		System.out.println("=================J2qjsScriptEngine=====================================");
		System.out.println("J2qjsScriptEngine.eval(path)=============");
		J2qjsScriptEngine jse=(J2qjsScriptEngine)se;
		System.out.println("tests/test_builtin.js");
		jse.eval(Paths.get("tests/test_builtin.js"),false);
		System.out.println("tests/test_closure.js");
		jse.eval(Paths.get("tests/test_closure.js"),false);
		System.out.println("tests/test_language.js");
		jse.eval(Paths.get("tests/test_language.js"),false);
		System.out.println("tests/test_loop.js");
		jse.eval(Paths.get("tests/test_loop.js"),false);
		//System.out.println("tests/microbench.js");
		//jse.eval(Paths.get("tests/microbench.js"),true);//OK to close. it is so long time 
		System.out.println("tests/test_bignum.js");
		jse.eval(Paths.get("tests/test_bignum.js"),true);
		//System.out.println("tests/test_bjson.js");
		//jse.eval(Paths.get("tests/test_bjson.js"),true)//failed for win32;
		System.out.println("tests/test_op_overloading.js");
		jse.eval(Paths.get("tests/test_op_overloading.js"),true);
		//System.out.println("tests/test_qjscalc.js");
		//jse.eval(Paths.get("tests/test_qjscalc.js"),true);//failed at Integer
		//System.out.println("tests/test_std.js");
		//jse.eval(Paths.get("tests/test_std.js"),true);//failed at tempfile
		//System.out.println("tests/test_worker.js");
		//jse.eval(Paths.get("tests/test_worker.js"),true);//failed at worker
		System.out.println("J2qjsScriptEngine.eval(script)===========");
		se.eval("console.log(JSON.stringify(std));");//dont try to eval modules by se.eval
		System.out.println("J2qjsScriptEngine.compile(path)==========");
		System.out.println("tests/test_builtin.js");
		jse.compile(Paths.get("tests/test_builtin.js"),false).eval();
		System.out.println("tests/test_closure.js");
		jse.compile(Paths.get("tests/test_closure.js"),false).eval();
		System.out.println("tests/test_language.js");
		jse.compile(Paths.get("tests/test_language.js"),false).eval();
		System.out.println("tests/test_loop.js");
		jse.compile(Paths.get("tests/test_loop.js"),false).eval();
		//System.out.println("tests/microbench.js");
		//jse.compile(Paths.get("tests/microbench.js"),true).eval();//OK to close. it is so long time 
		System.out.println("tests/test_bignum.js");
		jse.compile(Paths.get("tests/test_bignum.js"),true).eval();
		//System.out.println("tests/test_bjson.js");
		//jse.compile(Paths.get("tests/test_bjson.js"),true).eval();//failed for win32;
		System.out.println("tests/test_op_overloading.js");
		jse.compile(Paths.get("tests/test_op_overloading.js"),true).eval();
		//System.out.println("tests/test_qjscalc.js");
		//jse.compile(Paths.get("tests/test_qjscalc.js"),true).eval();//failed at Integer
		//System.out.println("tests/test_std.js");
		//jse.compile(Paths.get("tests/test_std.js"),true).eval();//failed at tempfile
		//System.out.println("tests/test_worker.js");
		//jse.compile(Paths.get("tests/test_worker.js"),true).eval();//failed at worker
		System.out.println("J2qjsScriptEngine.compile(script)========");
		Compilable cb=(Compilable)se;
		cb.compile("console.log(JSON.stringify(std));").eval();//dont try to compile modules by se.compile
		System.out.println("output of J2qjsScriptEngine.eval=========");
		int intvalue=(int)se.eval("12345;");
		System.out.println("intvalue="+intvalue);
		double doublevalue=(double)se.eval("12345.6;");
		System.out.println("doublevalue="+doublevalue);
		boolean booleanvalue=(boolean)se.eval("true;");
		System.out.println("booleanvalue="+booleanvalue);
		Integer integervalue=(Integer)se.eval("12345;");
		System.out.println("integervalue="+integervalue);
		long longvalue=(long)se.eval("12345n;");
		System.out.println("longvalue="+longvalue);
		//Object doublevalue2=se.eval("12345.6l;");
		//System.out.println("doublevalue2="+doublevalue2);//failed at float64;
		String stringvalue=(String)se.eval("'abcde';");
		System.out.println("stringvalue="+stringvalue);
		JSObject obj=(JSObject)se.eval("var x={a:1,b:2};x;");
		System.out.println("obj="+obj.stringify());
		obj.freeValue();//it is must to freevalue if you get a jsobject from eval.or memory leak.
		//JSObject obj2=(JSObject)se.eval("{a:1,b:2};");
		//System.out.println("obj="+obj2.stringify());//failed to get a value without name.
		//obj2.freeValue();
		JSArray ary=(JSArray)se.eval("var x=[123,456];x;");
		System.out.println("ary="+ary.stringify());
		ary.freeValue();
		JSFunction fnc=(JSFunction)se.eval("var f=function(a){console.log(a);};f;");
		fnc.call("i am a function;");
		fnc.freeValue();
		System.out.println("J2qjsScriptEngine.invokeMethod()=========");
		JSObject obj2=(JSObject)se.eval("var x={a:1,b:2,c:function(x,y){return this.a+this.b+x+y;}};x;");
		Invocable ic=(Invocable)se;
		int ret=(int)ic.invokeMethod(obj2, "c", 3,4);
		obj2.freeValue();
		System.out.println("ret="+ret);
		System.out.println("J2qjsScriptEngine.invokeFunction()=======");
		ic.invokeFunction("f", "i am a function;");
		System.out.println("J2qjsScriptEngine.put()==================");
		se.put("myattr", "hello");
		System.out.println("J2qjsScriptEngine.get()==================");
		System.out.println("myattr="+se.get("myattr"));
		System.out.println("J2qjsScriptEngine.createBindings()=======");
		Bindings bdg=se.createBindings();
		bdg.put("myattr", "world");
		System.out.println("J2qjsScriptEngine.setBindings()==========");
		se.setBindings(bdg, ScriptContext.ENGINE_SCOPE);
		System.out.println("myattr="+se.get("myattr"));
		System.out.println("=================J2qjsScriptContext====================================");
		System.out.println("ScriptContext.getAttribute()=============");
		ScriptContext sc=se.getContext();
		System.out.println("myattr="+sc.getAttribute("myattr"));
		System.out.println("myattr="+sc.getAttribute("myattr", ScriptContext.ENGINE_SCOPE));
		System.out.println("ScriptContext.getBindings()==============");
		System.out.println("myattr="+sc.getBindings(ScriptContext.ENGINE_SCOPE).get("myattr"));
		System.out.println("=================std===================================================");
		System.out.println("std.loadScript(filepath)=================");
		System.out.println("tests/test_builtin.js");
		se.eval("std.loadScript('tests/test_builtin.js');");
		System.out.println("tests/test_closure.js");
		se.eval("std.loadScript('tests/test_closure.js');");
		System.out.println("tests/test_language.js");
		se.eval("std.loadScript('tests/test_language.js');");
		System.out.println("tests/test_loop.js");
		se.eval("std.loadScript('tests/test_loop.js');");
		//System.out.println("tests/test_qjscalc.js");
		//_se.eval("std.loadScript('tests/test_qjscalc.js');");//failed at Integer
		System.out.println("std.loadModule(filepath)=================");
		//System.out.println("tests/microbench.js");
		//_se.eval("std.loadModule('tests/microbench.js');");//OK to close.
		System.out.println("tests/test_bignum.js");
		se.eval("std.loadModule('tests/test_bignum.js');");
		//System.out.println("tests/test_bjson.js");
		//_se.eval("std.loadModule('tests/test_bjson.js');");//failed for win32;
		System.out.println("tests/test_op_overloading.js");
		se.eval("std.loadModule('tests/test_op_overloading.js');");
		//System.out.println("tests/test_std.js");
		//_se.eval("std.loadModule('tests/test_std.js');");//failed at tempfile
		//System.out.println("tests/test_worker.js");
		//_se.eval("std.loadModule('tests/test_worker.js');");//failed at worker
		System.out.println("std.evalScript(script)===================");
		se.eval("std.evalScript('console.log(JSON.stringify(std))');");
		System.out.println("std.evalModule(script)===================");
		//import is the keyword for se.eval to decide to call in gobal type or module type.
		//so it must be i+mport in std.evalModule or it will be wrong.
		se.eval("std.evalModule(\"i\"+\"mport * as std3 from 'std';globalThis.std3 = std3;\");\n"
				+ "console.log(JSON.stringify(std3));");
		for(int i=0;i<10*100;i++) {
			se.eval("console.log('aaaaaa');");		
		}
	}
}
