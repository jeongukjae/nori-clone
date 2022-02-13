import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.analysis.TokenStream;
import org.apache.lucene.analysis.ko.GraphvizFormatter;
import org.apache.lucene.analysis.ko.KoreanTokenizer;
import org.apache.lucene.analysis.ko.KoreanTokenizer.DecompoundMode;
import org.apache.lucene.analysis.ko.POS;
import org.apache.lucene.analysis.ko.dict.ConnectionCosts;
import org.apache.lucene.analysis.ko.tokenattributes.PartOfSpeechAttribute;
import org.apache.lucene.analysis.tokenattributes.OffsetAttribute;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.nio.file.Paths;
import java.text.Normalizer;
import java.util.Scanner;

public class NoriVisualizer {
    public static void main(String[] args) {
        GraphvizFormatter dotOut = new GraphvizFormatter(ConnectionCosts.getInstance());
        Analyzer analyzer =
                new Analyzer() {
                    @Override
                    protected TokenStreamComponents createComponents(String fieldName) {
                        KoreanTokenizer tokenizer =
                                new KoreanTokenizer(
                                        TokenStream.DEFAULT_TOKEN_ATTRIBUTE_FACTORY,
                                        null,
                                        DecompoundMode.NONE,
                                        false,
                                        false);
                        tokenizer.setGraphvizFormatter(dotOut);
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

        // read
        Scanner scanner = new Scanner(System.in);
        String line = scanner.nextLine();

        // tokenize
        try {
            TokenStream tokenStream = analyzer.tokenStream("dummy", line);
            OffsetAttribute offsetAtt = tokenStream.addAttribute(OffsetAttribute.class);
            PartOfSpeechAttribute posAtt = tokenStream.addAttribute(PartOfSpeechAttribute.class);
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
        } catch (Exception e) {
            e.printStackTrace();
        }

        String filename;
        String workspace = System.getenv("BUILD_WORKSPACE_DIRECTORY");
        if (workspace == null) {
            filename = Paths.get(System.getProperty("user.dir"), "nori.dot").toString();
        } else {
            filename = Paths.get(workspace, "nori.dot").toString();
        }
        File file = new File(filename);

        try (BufferedWriter writer = new BufferedWriter(new FileWriter(file))) {
            writer.write(dotOut.finish());
        } catch (Exception e) {
            e.printStackTrace();
        }

        System.out.println("output file: " + filename);

        analyzer.close();
    }
}
