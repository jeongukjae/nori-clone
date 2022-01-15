import org.apache.lucene.analysis.TokenStream;
import org.apache.lucene.analysis.ko.KoreanAnalyzer;
import org.apache.lucene.analysis.ko.KoreanTokenizer;
import org.apache.lucene.analysis.ko.POS;
import java.io.IOException;
import org.apache.lucene.analysis.ko.tokenattributes.PartOfSpeechAttribute;
import org.apache.lucene.analysis.tokenattributes.CharTermAttribute;

import java.util.HashSet;
import java.util.Set;

public class NoriRunner {
    public static void main(String[] args) {
        Set<POS.Tag> stopTags = new HashSet<>();
        KoreanAnalyzer analyzer =
                new KoreanAnalyzer(null, KoreanTokenizer.DecompoundMode.DISCARD, stopTags, false);

        // TODO(jeongukjae): add analyzer code
        final TokenStream tokenStream = analyzer.tokenStream("dummy", "2018 평창 동게 올림픽");
        CharTermAttribute termAtt = tokenStream.addAttribute(CharTermAttribute.class);
        PartOfSpeechAttribute posAtt = tokenStream.addAttribute(PartOfSpeechAttribute.class);

        try {
            tokenStream.reset();
            while (tokenStream.incrementToken()) {
                System.out.println(
                        termAtt.toString()
                                + ", "
                                + posAtt.getPOSType().toString()
                                + ", "
                                + posAtt.getLeftPOS().toString()
                                + ", "
                                + posAtt.getRightPOS().toString());

                tokenStream.clearAttributes();
            }
            tokenStream.close();
        } catch (IOException e) {
			e.printStackTrace();
		}

        analyzer.close();
    }
}
