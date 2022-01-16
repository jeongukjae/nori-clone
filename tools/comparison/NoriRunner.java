import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.analysis.TokenStream;
import org.apache.lucene.analysis.Tokenizer;
import org.apache.lucene.analysis.ko.KoreanTokenizer;
import org.apache.lucene.analysis.ko.KoreanTokenizer.DecompoundMode;
import org.apache.lucene.analysis.ko.POS;
import org.apache.lucene.analysis.ko.tokenattributes.PartOfSpeechAttribute;
import org.apache.lucene.analysis.tokenattributes.OffsetAttribute;

import java.io.BufferedReader;
import java.io.FileReader;
import java.text.Normalizer;
import java.util.ArrayList;

public class NoriRunner {
    public static void main(String[] args) {
        Analyzer analyzer =
                new Analyzer() {
                    @Override
                    protected TokenStreamComponents createComponents(String fieldName) {
                        // keep punctuation and compound nouns
                        Tokenizer tokenizer =
                                new KoreanTokenizer(
                                        TokenStream.DEFAULT_TOKEN_ATTRIBUTE_FACTORY,
                                        null,
                                        DecompoundMode.NONE,
                                        false,
                                        false);
                        return new TokenStreamComponents(tokenizer, tokenizer);
                    }
                };

        try {
            TokenStream tokenStream = analyzer.tokenStream("dummy", "token");
            tokenStream.reset();
            while (tokenStream.incrementToken()) {}
            tokenStream.close();
        } catch (Exception e) {
            e.printStackTrace();
        }

        // read all
        ArrayList<String> lines = new ArrayList<String>();
        try {
            BufferedReader inFile = new BufferedReader(new FileReader(args[0]));
            String line;
            while ((line = inFile.readLine()) != null) {
                lines.add(line);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        // tokenize
        try {
            long start = System.currentTimeMillis();
            for (String line : lines) {
                TokenStream tokenStream = analyzer.tokenStream("dummy", line);
                OffsetAttribute offsetAtt = tokenStream.addAttribute(OffsetAttribute.class);
                PartOfSpeechAttribute posAtt =
                        tokenStream.addAttribute(PartOfSpeechAttribute.class);
                System.out.println(line);
                tokenStream.reset();
                while (tokenStream.incrementToken()) {
                    if (posAtt.getLeftPOS() == POS.Tag.SP && posAtt.getRightPOS() == POS.Tag.SP)
                        continue;
                    String token =
                            line.substring(offsetAtt.startOffset(), offsetAtt.endOffset()).trim();
                    System.out.println(
                            Normalizer.normalize(token, Normalizer.Form.NFKC)
                                    + ", "
                                    + posAtt.getPOSType().toString()
                                    + ", "
                                    + posAtt.getLeftPOS().toString()
                                    + ", "
                                    + posAtt.getRightPOS().toString());
                    tokenStream.clearAttributes();
                }
                tokenStream.close();
                System.out.println("");
            }
            long finish = System.currentTimeMillis();
            System.out.println("Elapsed time: " + (finish - start) + "ms");
        } catch (Exception e) {
            e.printStackTrace();
        }

        analyzer.close();
    }
}
