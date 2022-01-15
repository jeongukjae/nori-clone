import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.analysis.TokenStream;
import org.apache.lucene.analysis.Tokenizer;
import org.apache.lucene.analysis.ko.KoreanTokenizer;
import org.apache.lucene.analysis.ko.KoreanTokenizer.DecompoundMode;
import org.apache.lucene.analysis.ko.POS;
import org.apache.lucene.analysis.ko.tokenattributes.PartOfSpeechAttribute;
import org.apache.lucene.analysis.tokenattributes.OffsetAttribute;

import java.io.IOException;
import java.text.Normalizer;
import java.util.Scanner;

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

        Scanner sc = new Scanner(System.in);
        while (sc.hasNextLine()) {
            String input = sc.nextLine();
            TokenStream tokenStream = analyzer.tokenStream("dummy", input);
            OffsetAttribute offsetAtt = tokenStream.addAttribute(OffsetAttribute.class);
            PartOfSpeechAttribute posAtt = tokenStream.addAttribute(PartOfSpeechAttribute.class);
            System.out.println(input);
            try {
                tokenStream.reset();
                while (tokenStream.incrementToken()) {
                    if (posAtt.getLeftPOS() == POS.Tag.SP && posAtt.getRightPOS() == POS.Tag.SP)
                        continue;
                    String token =
                            input.substring(offsetAtt.startOffset(), offsetAtt.endOffset()).trim();
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
            } catch (IOException e) {
                e.printStackTrace();
            }
            System.out.println("");
        }

        analyzer.close();
    }
}
