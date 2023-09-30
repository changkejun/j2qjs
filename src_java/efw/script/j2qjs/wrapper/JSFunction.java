package efw.script.j2qjs.wrapper;

public class JSFunction extends JSObject {

    private final long objPointer;

    public JSFunction(QuickJSWrapper wrapper, long objPointer, long pointer) {
        super(wrapper, pointer);
        this.objPointer = objPointer;
    }

    public Object call(Object... args) {
        return wrapper.call(this, objPointer, args);
    }

}
