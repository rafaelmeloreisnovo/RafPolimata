package io.rafaelia.fieldlab;

import android.app.Activity;
import android.os.Bundle;
import android.os.Build;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

/**
 * Small Android platform edge. No third-party libraries and no permission.
 * All SHA3/PCM/WAV/DFT/ZIP calculations happen in freestanding C L0.
 * Android framework is the unavoidable hosted install/UI boundary.
 */
public final class FieldLabActivity extends Activity {
    private TextView result;
    private boolean nativeLoaded;

    private native int nativeSelfTest();

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        LinearLayout content = new LinearLayout(this);
        content.setOrientation(LinearLayout.VERTICAL);
        content.setPadding(24, 24, 24, 24);
        TextView title = new TextView(this);
        title.setText("RAFAELIA | Laboratorio L0");
        title.setTextSize(20);
        content.addView(title);
        TextView detail = new TextView(this);
        detail.setText("SHA3-256 / PCM16 48 kHz / WAV / DFT / ZIP32\n"
            + "ABI: " + Build.CPU_ABI + " | Android API: " + Build.VERSION.SDK_INT
            + "\nNucleo C freestanding; interface Android minima.\n"
            + "Sem rede, microfone, arquivos privados ou permissões.");
        content.addView(detail);
        result = new TextView(this);
        result.setTextSize(16);
        result.setTextIsSelectable(true);
        content.addView(result);
        Button run = new Button(this);
        run.setText("Executar cinco testes L0");
        content.addView(run);
        run.setOnClickListener(new View.OnClickListener() {
            @Override public void onClick(View view) { runTests(); }
        });
        ScrollView scroller = new ScrollView(this);
        scroller.addView(content);
        setContentView(scroller);
        try {
            System.loadLibrary("raffieldlab");
            nativeLoaded = true;
            result.setText("Biblioteca nativa carregada. Teste automatico:");
            runTests();
        } catch (Throwable error) {
            nativeLoaded = false;
            result.setText("Falha ao carregar modulo nativo (NAO PASS): "
                + error.getClass().getSimpleName());
        }
    }

    private void runTests() {
        if (!nativeLoaded) {
            result.setText("BLOQUEADO: modulo nativo nao carregado.");
            return;
        }
        try {
            final int mask = nativeSelfTest();
            final String[] names = {
                "SHA3-256 abc", "PCM16 48000 Hz", "WAV 44B",
                "DFT 8 bins", "ZIP32 one entry"
            };
            StringBuilder sb = new StringBuilder();
            sb.append(mask == 31 ? "PASS_HOSTED_ON_DEVICE_L0_SELFTEST" : "FAIL_CLOSED")
              .append("\nbitmask=").append(mask).append("/31");
            for (int i = 0; i < names.length; i++) {
                sb.append("\n").append(names[i]).append(": ")
                  .append((mask & (1 << i)) != 0 ? "PASS" : "FAIL");
            }
            sb.append("\nSem afirmacao de validade fisica do audio.");
            result.setText(sb.toString());
        } catch (Throwable error) {
            result.setText("Falha na ponte nativa: "
                + error.getClass().getSimpleName());
        }
    }
}
