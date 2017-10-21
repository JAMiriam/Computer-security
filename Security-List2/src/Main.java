public class Main {
    public static void main(String[] args) {
        MessageDecryptor decryptor = new MessageDecryptor();
        decryptor.setCryptograms("data.txt");
        decryptor.establishKey();
        decryptor.printDecrypted();
    }
}
