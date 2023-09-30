package efw.script.j2qjs.wrapper;

public class JSArray extends JSObject {

    public JSArray(QuickJSWrapper wrapper, long pointer) {
        super(wrapper, pointer);
    }

    public int length() {
        return wrapper.length(this);
    }

    public Object get(int index) {
        return wrapper.get(this, index);
    }

    public void set(Object value, int index) {
    	wrapper.set(this, value, index);
    }

}
