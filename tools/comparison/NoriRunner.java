import org.apache.lucene.analysis.ko.KoreanAnalyzer;
import org.apache.lucene.analysis.ko.KoreanTokenizer;
import org.apache.lucene.analysis.ko.POS;
import java.util.HashSet;
import java.util.Set;

public class NoriRunner {
  public static void main (String[] args) {
    Set<POS.Tag> stopTags = new HashSet<>();
    KoreanAnalyzer analyzer = new KoreanAnalyzer(null, KoreanTokenizer.DecompoundMode.DISCARD, stopTags, false);

    // TODO(jeongukjae): add analyzer code

    analyzer.close();
  }
}
