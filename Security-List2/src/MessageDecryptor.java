import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

public class MessageDecryptor {
    private ArrayList<Cryptogram> cryptograms;
    private HashMap<Integer, Integer> candidateLetters;
    private HashMap<Integer, Integer> candidateKey;

    public MessageDecryptor() {
        cryptograms = new ArrayList<>();
        candidateLetters = new HashMap<>();
        generateLetters();
    }

    public ArrayList<Integer> establishKey() {
        ArrayList<Integer> key = new ArrayList<>();

        for (int i = 0; i < longestCryptogramSize(); i++) {
            candidateKey = new HashMap<>();
            ArrayList<Cryptogram> activeCryptograms = activeCryptograms(cryptograms, i);
            for (Cryptogram cryptogram : activeCryptograms) {
                for (Integer candidate : candidateLetters.keySet()) {
                    Pair<Integer, Integer> tempKey =
                            new Pair<>(cryptogram.getCharacter(i) ^ candidate, candidateLetters.get(candidate));
                    if (!candidateKey.containsKey(tempKey.first)) {
                        candidateKey.put(tempKey.first, tempKey.second);
                    } else
                        candidateKey.put(tempKey.first, candidateKey.get(tempKey.first)+candidateLetters.get(candidate));
                }
            }
            candidateKey = MapUtil.sortByValue(candidateKey);
            key.add(findBestKey(i));
        }
        return key;
    }

    private int findBestKey(int position) {
        Integer maxCandidate = (int) ' ';
        Integer maxCounter = 0;
        ArrayList<Cryptogram> activeCryptograms = activeCryptograms(cryptograms, position);
        for (Integer candidate : candidateKey.keySet()) {
            int counter = 0;
            for (Cryptogram cryptogram : activeCryptograms) {
                if (isInAlphabet(cryptogram.getCharacter(position) ^ candidate))
                    counter++;
            }
            if(counter>maxCounter) {
                maxCounter = counter;
                maxCandidate = candidate;
            }
            if (counter >= cryptograms.size()) return candidate;
        }
        return maxCandidate;
    }

    private ArrayList<Cryptogram> activeCryptograms(ArrayList<Cryptogram> cryptograms, int position) {
        ArrayList<Cryptogram> active = new ArrayList<>();
        for(Cryptogram cryptogram : cryptograms) {
            if(position<cryptogram.size())
                active.add(cryptogram);
        }
        return active;
    }

    private boolean isInAlphabet(int checkedLetter) {
        for (Integer letter : candidateLetters.keySet()) {
            if (letter == checkedLetter)
                return true;
        }
        return false;
    }

    private int longestCryptogramSize() {
        int longestSize = 0;
        for(Cryptogram cryptogram : cryptograms) {
            if(cryptogram.size()>longestSize)
                longestSize = cryptogram.size();
        }
        return longestSize;
    }

    public void printDecrypted() {
        ArrayList<Integer> key = establishKey();
        for (Cryptogram cryptogram : cryptograms) {
            for (int i = 0 ; i < cryptogram.size(); i++) {
                System.out.print((char) (cryptogram.getCharacter(i) ^ key.get(i)));
            }
            System.out.println();
        }
    }

    public void setCryptograms(String filepath) {
        try (BufferedReader reader = new BufferedReader(new FileReader(filepath))) {
            String line;
            Cryptogram cryptogram;
            while ((line = reader.readLine()) != null) {
                cryptogram = new Cryptogram(line);
                cryptograms.add(cryptogram);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void generateLetters() {
        candidateLetters.put((int)' ', 100);
        candidateLetters.put((int)'a', 89);
        candidateLetters.put((int)'i', 82);
        candidateLetters.put((int)'o', 78);
        candidateLetters.put((int)'e', 77);
        candidateLetters.put((int)'z', 56);
        candidateLetters.put((int)'n', 55);
        candidateLetters.put((int)'r', 47);
        candidateLetters.put((int)'w', 47);
        candidateLetters.put((int)'s', 43);
        candidateLetters.put((int)'t', 40);
        candidateLetters.put((int)'c', 40);
        candidateLetters.put((int)'y', 38);
        candidateLetters.put((int)'k', 35);
        candidateLetters.put((int)'d', 33);
        candidateLetters.put((int)'p', 31);
        candidateLetters.put((int)'m', 28);
        candidateLetters.put((int)'u', 25);
        candidateLetters.put((int)'j', 23);
        candidateLetters.put((int)'l', 21);
        candidateLetters.put((int)'b', 15);
        candidateLetters.put((int)'g', 14);
        candidateLetters.put((int)'h', 11);
        candidateLetters.put((int)'f', 3);
        candidateLetters.put((int)'q', 1);
        candidateLetters.put((int)'v', 1);
        candidateLetters.put((int)'x', 1);
        candidateLetters.put((int)',', 10);
        candidateLetters.put((int)'.', 10);
        candidateLetters.put((int)'-', 10);
        candidateLetters.put((int)'"', 10);
        candidateLetters.put((int)'!', 10);
        candidateLetters.put((int)'?', 10);
        candidateLetters.put((int)':', 10);
        candidateLetters.put((int)';', 10);
        candidateLetters.put((int)'(', 10);
        candidateLetters.put((int)')', 10);

        for (int i = 65; i < 91; i++) {
            candidateLetters.put(i, 10);
        }
        for (int i = 48; i <= 57; i++) {
            candidateLetters.put(i, 10);
        }

    }
}
