import java.util.ArrayList;

public class Cryptogram {
    private ArrayList<Integer> characters;

    public Cryptogram(String cryptogram) {
        this.characters = new ArrayList<>();
        String d = " ";
        for (String character : cryptogram.split(d)) {
            characters.add(Integer.parseInt(character, 2));
        }
    }

    public int getCharacter(int i) {
        if(characters.size()>i)
            return characters.get(i);
        else
            return (int) ' ';
    }

    public int size() {
        return characters.size();
    }
}

