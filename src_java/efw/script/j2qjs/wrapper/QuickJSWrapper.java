package efw.script.j2qjs.wrapper;

import java.util.HashMap;
/**
 * The Wrapper for QuickJS
 * @author kejun.chang
 */
public class QuickJSWrapper {
    static {
        System.loadLibrary("libj2qjs");
    }
	private static final String UNKNOWN_FILE = "unknown.js";
	//=========================================================================
	//create and destory
	//=========================================================================
    private final long runtime;
    private final long context;
    public QuickJSWrapper() {
        try {
            runtime = createRuntime();
            context = createContext(runtime);
        } catch (UnsatisfiedLinkError e) {
            throw new QuickJSException("Failed to load the libj2qjs library.");
        }
    }
    public void destroy() {
        callFunctionMap.clear();
        destroyContext(context);
    }
    public void runGC() {
        runGC(runtime);
    }
	//=========================================================================
    //
	//=========================================================================

    private final HashMap<Integer, JSCallFunction> callFunctionMap = new HashMap<>();

	//=========================================================================
    //communication to QuickJS
	//=========================================================================
    
    public Object evaluate(String script) {
        return evaluate(script, UNKNOWN_FILE);
    }

    public Object evaluate(String script, String fileName) {
    	//TODO
    	if (script.indexOf("import")==-1) {
            return evaluate(context, script, fileName);
    	}else {
            return evaluateModule(context, script, fileName);
    	}
    }
	//=========================================================================
    public byte[] compile(String source) {
        return compile(source, UNKNOWN_FILE);
    }

    public byte[] compile(String script, String fileName) {
    	if (script.indexOf("import")==-1) {
            return compile(context, script, fileName, false);
    	}else {
            return compile(context, script, fileName, true);
        }
    }
    public Object execute(byte[] code) {
        return execute(context, code);
    }
	//=========================================================================
    //
	//=========================================================================
    public JSObject getGlobalObject() {
        return getGlobalObject(context);
    }
    public Object getProperty(JSObject jsObj, String name) {
        return getProperty(context, jsObj.getPointer(), name);
    }
    public void setProperty(JSObject jsObj, String name, Object value) {
        if (value instanceof JSCallFunction) {
            // Todo 优化：可以只传 callFunctionId 给到 JNI.
            putCallFunction((JSCallFunction) value);
        }
        setProperty(context, jsObj.getPointer(), name, value);
    }
    public int length(JSArray jsArray) {
        return length(context, jsArray.getPointer());
    }
    public Object get(JSArray jsArray, int index) {
        return get(context, jsArray.getPointer(), index);
    }
    protected void set(JSArray jsArray, Object value, int index) {
        set(context, jsArray.getPointer(), value, index);
    }
    public void freeValue(JSObject jsObj) {
        freeValue(context, jsObj.getPointer());
    }
	//=========================================================================
	//=========================================================================
    public String stringify(JSObject jsObj) {
        return stringify(context, jsObj.getPointer());
    }

    public JSObject createNewJSObject() {
        return (JSObject) parseJSON(context, "{}");
    }

    public JSArray createNewJSArray() {
        return (JSArray) parseJSON(context, "[]");
    }
	//=========================================================================

    private void putCallFunction(JSCallFunction callFunction) {
        int callFunctionId = callFunction.hashCode();
        callFunctionMap.put(callFunctionId, (JSCallFunction) callFunction);
    }

    /**
     * 该方法只提供给 Native 层回调.
     * @param callFunctionId JSCallFunction 对象标识
     */
    public void removeCallFunction(int callFunctionId) {
        callFunctionMap.remove(callFunctionId);
    }

    /**
     * 该方法只提供给 Native 层回调.
     * @param callFunctionId JSCallFunction 对象标识
     * @param args JS 到 Java 的参数映射
     */
    public Object callFunctionBack(int callFunctionId, Object... args) {
        JSCallFunction callFunction = callFunctionMap.get(callFunctionId);
        Object ret = callFunction.call(args);
        if (ret instanceof JSCallFunction) {
            putCallFunction((JSCallFunction) ret);
        }
        return ret;
    }

    Object call(JSObject func, long objPointer, Object... args) {
        for (int i = 0; i < args.length; i++) {
            Object arg = args[i];
            if (arg instanceof JSCallFunction) {
                putCallFunction((JSCallFunction) arg);
            }
        }
        return call(context, func.getPointer(), objPointer, args);
    }

    // runtime context
    private native long createRuntime();
    private native long createContext(long runtime);
    private native void destroyContext(long context);
    private native void runGC(long runtime);

    private native Object evaluate(long context, String script, String fileName);
    private native Object evaluateModule(long context, String script, String fileName);  
    private native byte[] compile(long context, String sourceCode, String fileName, boolean isModule); // Bytecode compile
    private native Object execute(long context, byte[] bytecode); // Bytecode execute
    
    private native JSObject getGlobalObject(long context);
    private native Object getProperty(long context, long objValue, String name);
    private native void setProperty(long context, long objValue, String name, Object value);
    private native int length(long context, long objValue);
    private native Object get(long context, long objValue, int index);
    private native void set(long context, long objValue, Object value, int index);
    private native void freeValue(long context, long objValue);

    private native Object call(long context, long func, long thisObj, Object[] args);
    private native String stringify(long context, long objValue);
    private native Object parseJSON(long context, String json);

}
