package efw.script.j2qjs.wrapper;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Arrays;

/**
 * The Object from QuickJS context.
 * @author kejun.chang
 *
 */
public class JSObject {

    protected final QuickJSWrapper wrapper;
    private final long pointer;

    protected JSObject(QuickJSWrapper wrapper, long pointer) {
        this.wrapper = wrapper;
        this.pointer = pointer;
    }
    public void freeValue() {
    	wrapper.freeValue(this);
    }

    protected long getPointer() {
        return pointer;
    }
    public JSArray getNames() {
        JSFunction getOwnPropertyNames = (JSFunction) wrapper.evaluate("Object.getOwnPropertyNames");
        return (JSArray) getOwnPropertyNames.call(this);
    }
    //=========================================================================
    //setProperty and other similar functions
    //=========================================================================
    public void setProperty(String name, String value) {
    	wrapper.setProperty(this, name, value);
    }

    public void setProperty(String name, int value) {
    	wrapper.setProperty(this, name, value);
    }

    public void setProperty(String name, long value) {
    	wrapper.setProperty(this, name, value);
    }

    public void setProperty(String name, JSObject value) {
    	wrapper.setProperty(this, name, value);
    }

    public void setProperty(String name, boolean value) {
    	wrapper.setProperty(this, name, value);
    }

    public void setProperty(String name, double value) {
    	wrapper.setProperty(this, name, value);
    }

    public void setProperty(String name, JSCallFunction value) {
    	wrapper.setProperty(this, name, value);
    }
    
    @SuppressWarnings("rawtypes")
	public void setProperty(String name, Object javaObj, Class clazz) {
        JSObject jsObj = wrapper.createNewJSObject();
        Method[] methods = clazz.getMethods();
        for (Method method : methods) {
            Object finalJavaObj = javaObj;
            jsObj.setProperty(method.getName(), args -> {
                try {
                    return method.invoke(finalJavaObj, args);
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                } catch (InvocationTargetException e) {
                    e.printStackTrace();
                }
                return null;
            });
        }

        setProperty(name, jsObj);
    }
    //=========================================================================
    //getProperty and other similar functions
    //=========================================================================
    public Object getProperty(String name) {
        return wrapper.getProperty(this, name);
    }
    public String getString(String name) {
        Object value = getProperty(name);
        return value instanceof String ? (String) value : null;
    }

    public Integer getInteger(String name) {
        Object value = getProperty(name);
        return value instanceof Integer ? (Integer) value : null;
    }

    public Boolean getBoolean(String name) {
        Object value = getProperty(name);
        return value instanceof Boolean ? (Boolean) value : null;
    }

    public Double getDouble(String name) {
        Object value = getProperty(name);
        return value instanceof Double ? (Double) value : null;
    }

    public Long getLong(String name) {
        Object value = getProperty(name);
        return value instanceof Long ? (Long) value : null;
    }

    public JSObject getJSObject(String name) {
        Object value = getProperty(name);
        return value instanceof JSObject ? (JSObject) value : null;
    }

    public JSFunction getJSFunction(String name) {
        Object value = getProperty(name);
        return value instanceof JSFunction ? (JSFunction) value : null;
    }

    public JSArray getJSArray(String name) {
        Object value = getProperty(name);
        return value instanceof JSArray ? (JSArray) value : null;
    }

    //=========================================================================
    //Overrided Java Object functions
    //=========================================================================
    @Override
    public String toString() {
        JSFunction toString = getJSFunction("toString");
        return (String) toString.call();
    }

    public String stringify() {
        return wrapper.stringify(this);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        JSObject jsObject = (JSObject) o;
        return pointer == jsObject.pointer;
    }

    @Override
    public int hashCode() {
        return Arrays.hashCode(new long[]{pointer});
    }
    
}
