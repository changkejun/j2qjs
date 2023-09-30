package efw.script.j2qjs.engine;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.Reader;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Path;

import javax.script.AbstractScriptEngine;
import javax.script.Bindings;
import javax.script.Compilable;
import javax.script.CompiledScript;
import javax.script.Invocable;
import javax.script.ScriptContext;
import javax.script.ScriptEngine;
import javax.script.ScriptEngineFactory;
import javax.script.ScriptException;
import javax.script.SimpleBindings;

import efw.script.j2qjs.wrapper.JSFunction;
import efw.script.j2qjs.wrapper.JSObject;
import efw.script.j2qjs.wrapper.QuickJSWrapper;
/**
 * The ScriptEngine for j2qjs
 * @author kejun.chang
 *
 */
public class J2qjsScriptEngine extends AbstractScriptEngine implements Invocable,Compilable,ScriptEngine {
    // the factory that created this engine
    private final ScriptEngineFactory factory;
	@Override
	public ScriptEngineFactory getFactory() {
		return factory;
	}
	J2qjsScriptEngine(final J2qjsScriptEngineFactory factory) {
		this.context=new J2qjsScriptContext();
        this.factory = factory;
    }
	private QuickJSWrapper getWrapper() {
		J2qjsScriptContext context=(J2qjsScriptContext)this.context;
		return context.wrapper;
	}
	/////////////////////////////////////////////////////
	@Override
	public Object eval(String script, ScriptContext context) throws ScriptException {
		if (context==null || context==this.context) {
			return this.getWrapper().evaluate(script);
		}else {
			throw new ScriptException("The context must be the orginal one created with the engine.");
		}
	}
	@Override
	public Object eval(Reader reader, ScriptContext context) throws ScriptException {
		try {
			BufferedReader breader=new BufferedReader(reader);
			StringBuilder scriptb=new StringBuilder();
			String row;
			while((row=breader.readLine())!=null) {
				scriptb.append(row);
				scriptb.append("\n");
			}
			breader.close();
	        return eval(scriptb.toString(),context);
		} catch (IOException e) {
			throw new ScriptException(e.getMessage());
		}
	}
	public Object eval(Path path,boolean isMudule) throws ScriptException, IOException {
		String script=new String(Files.readAllBytes(path), Charset.forName("UTF-8"));
		return ((J2qjsScriptContext) context).wrapper.evaluate(script,path.getFileName().toString(),isMudule);
	}
	/////////////////////////////////////////////////////
	@Override
	public CompiledScript compile(String script) throws ScriptException {
		byte[] complied=getWrapper().compile(script);
        return new CompiledScript() {
            @Override
            public Object eval(final ScriptContext context) throws ScriptException {
        		if (context==null || context==J2qjsScriptEngine.this.context) {
        			return J2qjsScriptEngine.this.getWrapper().execute(complied);
        		}else {
        			throw new ScriptException("The context must be the orginal one created with the engine.");
        		}
            }
            @Override
            public ScriptEngine getEngine() {
                return J2qjsScriptEngine.this;
            }
        };
	}
	@Override
	public CompiledScript compile(Reader reader) throws ScriptException {
		try {
			BufferedReader breader=new BufferedReader(reader);
			StringBuilder scriptb=new StringBuilder();
			String row;
			while((row=breader.readLine())!=null) {
				scriptb.append(row);
				scriptb.append("\n");
			}
			breader.close();
	        return compile(scriptb.toString());
		} catch (IOException e) {
			throw new ScriptException(e.getMessage());
		}
	}
	public CompiledScript compile(Path path,boolean isMudule) throws ScriptException, IOException {
		String script=new String(Files.readAllBytes(path), Charset.forName("UTF-8"));
		String fileName=path.getFileName().toString();
		byte[] complied=getWrapper().compile(script,fileName,isMudule);
        return new CompiledScript() {
            @Override
            public Object eval(final ScriptContext context) throws ScriptException {
        		if (context==null || context==J2qjsScriptEngine.this.context) {
        			return J2qjsScriptEngine.this.getWrapper().execute(complied);
        		}else {
        			throw new ScriptException("The context must be the orginal one created with the engine.");
        		}
            }
            @Override
            public ScriptEngine getEngine() {
                return J2qjsScriptEngine.this;
            }
        };
	}
	///////////////////////////////////////////////////
	@Override
	public Object invokeMethod(Object thiz, String name, Object... args) throws ScriptException, NoSuchMethodException {
		JSObject jsobj=(JSObject)thiz;
		JSFunction jsfnc =jsobj.getJSFunction(name);
		try {
			return jsfnc.call(args);
		}finally {
			jsfnc.freeValue();
			//jsobj can not be free here. you should do it by youself.
		}
	}
	@Override
	public Object invokeFunction(String name, Object... args) throws ScriptException, NoSuchMethodException {
		JSObject jsobj=getWrapper().getGlobalObject();
		JSFunction jsfnc =jsobj.getJSFunction(name);
		try {
			return jsfnc.call(args);
		}finally {
			jsobj.freeValue();
			jsfnc.freeValue();
		}
	}
	///////////////////////////////////////////////////
	@Override
	public Bindings createBindings() {
		SimpleBindings sbs=new SimpleBindings();
		return sbs;
	}
	@Override
	public <T> T getInterface(Class<T> clasz) {
		return null;//TODO I give up to do it.
	}
	@Override
	public <T> T getInterface(Object thiz, Class<T> clasz) {
		return null;//TODO I give up to do it.
	}
}
