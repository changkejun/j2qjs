package efw.script.j2qjs.engine;

import java.util.Arrays;
import java.util.List;
import java.util.Objects;

import javax.script.ScriptEngine;
import javax.script.ScriptEngineFactory;
/**
 * The ScriptEngineFactory for j2qjs
 * @author kejun.chang
 *
 */
public final class J2qjsScriptEngineFactory implements ScriptEngineFactory {

	@Override
	public String getEngineName() {
		return "j2qjs";
	}

	@Override
	public String getEngineVersion() {
		return "0.1";//TODO it should be something from the project
	}

	@Override
	public List<String> getExtensions() {
		return names;
	}

    private static final List<String> mimeTypes = Arrays.asList(
            "application/javascript",
            "application/ecmascript",
            "text/javascript",
            "text/ecmascript"
        );
	@Override
	public List<String> getMimeTypes() {
		return mimeTypes;
	}

    private static final List<String> names = Arrays.asList(
    		"j2qjs","J2QJS","quickjs", "QuickJS",
            "js", "JS",
            "JavaScript", "javascript",
            "ECMAScript", "ecmascript"
        );
	@Override
	public List<String> getNames() {
		return names;
	}

	@Override
	public String getLanguageName() {
		return "ECMAScript";
	}

	@Override
	public String getLanguageVersion() {
		return "ECMA - 262 Edition 11";
	}

	@Override
	public Object getParameter(String key) {
        switch (key) {
        case ScriptEngine.NAME:
            return "javascript";
        case ScriptEngine.ENGINE:
            return getEngineName();
        case ScriptEngine.ENGINE_VERSION:
            return getEngineVersion();
        case ScriptEngine.LANGUAGE:
            return getLanguageName();
        case ScriptEngine.LANGUAGE_VERSION:
            return getLanguageVersion();
        default:
            return null;
        }
	}

	@Override
	public String getMethodCallSyntax(String obj, String method, String... args) {
        final StringBuilder sb = new StringBuilder().
                append(Objects.requireNonNull(obj)).append('.').
                append(Objects.requireNonNull(method)).append('(');
            final int len = args.length;

            if (len > 0) {
                sb.append(Objects.requireNonNull(args[0]));
            }
            for (int i = 1; i < len; i++) {
                sb.append(',').append(Objects.requireNonNull(args[i]));
            }
            sb.append(')');

            return sb.toString();
	}

    /**
     * Implementation of the Quote(value) operation as defined in the ECMAscript
     * spec. It wraps a String value in double quotes and escapes characters
     * within.
     *
     * @param value string to quote
     *
     * @return quoted and escaped string
     */
    private static String quote(final String value) {

        final StringBuilder product = new StringBuilder();

        product.append("\"");

        for (final char ch : value.toCharArray()) {
            switch (ch) {
            case '\\':
                product.append("\\\\");
                break;
            case '"':
                product.append("\\\"");
                break;
            case '\b':
                product.append("\\b");
                break;
            case '\f':
                product.append("\\f");
                break;
            case '\n':
                product.append("\\n");
                break;
            case '\r':
                product.append("\\r");
                break;
            case '\t':
                product.append("\\t");
                break;
            default:
                if (ch < ' ') {
                    product.append(unicodeEscape(ch));
                    break;
                }

                product.append(ch);
                break;
            }
        }

        product.append("\"");

        return product.toString();
    }
    private static String unicodeEscape(final char ch) {
        final StringBuilder sb = new StringBuilder();

        sb.append("\\u");

        final String hex = Integer.toHexString(ch);
        for (int i = hex.length(); i < 4; i++) {
            sb.append('0');
        }
        sb.append(hex);

        return sb.toString();
    }    
	@Override
	public String getOutputStatement(String toDisplay) {
		return "print(" + quote(toDisplay) + ")";
	}

	@Override
	public String getProgram(String... statements) {
        Objects.requireNonNull(statements);
        final StringBuilder sb = new StringBuilder();

        for (final String statement : statements) {
            sb.append(Objects.requireNonNull(statement)).append(';');
        }

        return sb.toString();
	}

 	@Override
	public ScriptEngine getScriptEngine() {
        try {
            return new J2qjsScriptEngine(this);
        } catch (final RuntimeException e) {
            throw e;
        }
	}
 	
}
