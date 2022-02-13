import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.analysis.TokenStream;
import org.apache.lucene.analysis.Tokenizer;
import org.apache.lucene.analysis.ko.KoreanTokenizer;
import org.apache.lucene.analysis.ko.KoreanTokenizer.DecompoundMode;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.List;

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
        List<String> cases = lines.subList(0, Integer.parseInt(args[1]));

        // tokenize
        try {
            long start = System.currentTimeMillis();
            for (String line : cases) {
                TokenStream tokenStream = analyzer.tokenStream("dummy", line);
                tokenStream.reset();
                while (tokenStream.incrementToken())
                    ;
                tokenStream.close();
            }
            long finish = System.currentTimeMillis();
            System.out.println((finish - start) + "ms");
        } catch (Exception e) {
            e.printStackTrace();
        }

        analyzer.close();
    }
}
