package efw.script.j2qjs.engine;

import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.Reader;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Objects;

import javax.script.Bindings;
import javax.script.ScriptContext;
import javax.script.SimpleBindings;

import efw.script.j2qjs.wrapper.JSObject;
import efw.script.j2qjs.wrapper.QuickJSWrapper;
/**
 * The ScriptContext for j2qjs
 * @author kejun.chang
 *
 */
public class J2qjsScriptContext implements ScriptContext {
    protected Writer writer;
    protected Writer errorWriter;
    protected Reader reader;
    protected Bindings engineScope;
    protected Bindings globalScope;//TODO now the globalScope has not been used
    private static List<Integer> scopes;
    static {
        scopes = new ArrayList<Integer>(2);
        scopes.add(ENGINE_SCOPE);
        scopes.add(GLOBAL_SCOPE);
        scopes = Collections.unmodifiableList(scopes);
    }
    protected final QuickJSWrapper wrapper;
    /**
     * 
     */
	protected J2qjsScriptContext() {
        engineScope = new SimpleBindings();
        globalScope = null;
        reader = new InputStreamReader(System.in);
        writer = new PrintWriter(System.out , true);
        errorWriter = new PrintWriter(System.err, true);
        wrapper=new QuickJSWrapper();
        wrapper.getGlobalObject().getJSObject("console").setProperty("stdout", args -> {
            if (args.length == 2) {
                String level = (String) args[0];
                String info = (String) args[1];
                switch (level) {
                    case "info":
                        System.out.println("[info]"+info);
                        break;
                    case "warn":
                        System.out.println("[warn]"+info);
                        break;
                    case "error":
                        System.err.println("[error]"+info);
                        break;
                    case "log":
                    case "debug":
                    default:
                        System.out.println(info);
                        break;
                }
            }

            return null;
        });
	}

	@Override
	public void setBindings(Bindings bindings, int scope) {
        switch (scope) {
	        case ENGINE_SCOPE:
	            if (bindings == null) {
	                throw new NullPointerException("Engine scope cannot be null.");
	            }
	            engineScope = bindings;
            	JSObject g=(JSObject)wrapper.getGlobalObject();
            	for(Map.Entry<String, Object> entry : bindings.entrySet()) {
            		String key=entry.getKey();
            		Object value=entry.getValue();
            		this.wrapper.setProperty(g, key, value);
            	}
	            g.freeValue();
	            break;
	        case GLOBAL_SCOPE:
	            globalScope = bindings;
	            break;
	        default:
	            throw new IllegalArgumentException("Invalid scope value.");
        }
	}

	@Override
	public Bindings getBindings(int scope) {//////////////////////
        if (scope == ENGINE_SCOPE) {
            return engineScope;
        } else if (scope == GLOBAL_SCOPE) {
            return globalScope;
        } else {
            throw new IllegalArgumentException("Illegal scope value.");
        }
	}

	@Override
	public void setAttribute(String name, Object value, int scope) {//////////////////////
        checkName(name);
        switch (scope) {

            case ENGINE_SCOPE:
                engineScope.put(name, value);
            	JSObject g=(JSObject)wrapper.getGlobalObject();
            	this.wrapper.setProperty(g, name, value);
            	g.freeValue();
                return;

            case GLOBAL_SCOPE:
                if (globalScope != null) {
                    globalScope.put(name, value);
                }
                return;

            default:
                throw new IllegalArgumentException("Illegal scope value.");
        }
	}

	@Override
	public Object getAttribute(String name, int scope) {//////////////////////
        checkName(name);
        switch (scope) {

            case ENGINE_SCOPE:
                return engineScope.get(name);

            case GLOBAL_SCOPE:
                if (globalScope != null) {
                    return globalScope.get(name);
                }
                return null;

            default:
                throw new IllegalArgumentException("Illegal scope value.");
        }
	}

	@Override
	public Object removeAttribute(String name, int scope) {//////////////////////
        checkName(name);
        switch (scope) {

            case ENGINE_SCOPE:
                if (getBindings(ENGINE_SCOPE) != null) {
                    return getBindings(ENGINE_SCOPE).remove(name);
                }
                return null;

            case GLOBAL_SCOPE:
                if (getBindings(GLOBAL_SCOPE) != null) {
                    return getBindings(GLOBAL_SCOPE).remove(name);
                }
                return null;

            default:
                throw new IllegalArgumentException("Illegal scope value.");
        }
	}

	@Override
	public Object getAttribute(String name) {//////////////////////
        checkName(name);
        if (engineScope.containsKey(name)) {
            return getAttribute(name, ENGINE_SCOPE);
        } else if (globalScope != null && globalScope.containsKey(name)) {
            return getAttribute(name, GLOBAL_SCOPE);
        }

        return null;
	}
    private void checkName(String name) {
        Objects.requireNonNull(name);
        if (name.isEmpty()) {
            throw new IllegalArgumentException("name cannot be empty");
        }
    }

	@Override
	public int getAttributesScope(String name) {
        checkName(name);
        if (engineScope.containsKey(name)) {
            return ENGINE_SCOPE;
        } else if (globalScope != null && globalScope.containsKey(name)) {
            return GLOBAL_SCOPE;
        } else {
            return -1;
        }
	}

	@Override
	public Writer getWriter() {
        return writer;
	}

	@Override
	public Writer getErrorWriter() {
        return errorWriter;
	}

	@Override
	public void setWriter(Writer writer) {
        this.writer = writer;
	}

	@Override
	public void setErrorWriter(Writer writer) {
        this.errorWriter = writer;
	}

	@Override
	public Reader getReader() {
        return reader;
	}

	@Override
	public void setReader(Reader reader) {
        this.reader = reader;
	}

	@Override
	public List<Integer> getScopes() {
        return scopes;
	}

}
