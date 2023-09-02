import sys
import soundfile as sf
import speech_recognition as sr
import time
import os

min_duration = 20
name_file_audio = sys.argv[1]
duration = sys.argv[2]

class Data:
    def __init__(self):
        self.audio = None
        self.recognised = False
        self.text = ""
        self.start = None
        self.stop = None
    def copy(self):
        newData = Data()
        newData.audio = self.audio.copy()
        newData.recognised = self.recognised
        newData.text = self.text
        newData.start = self.start
        newData.stop = self.stop
        return newData

def fill_audio(name):
    list_data = []
    data_in, samplerate = sf.read(name)
    data = Data()
    data.audio = data_in
    data.start = 0
    data.stop = len(data_in)
    list_data.append(data)
    return list_data, samplerate 

def time_trad(sec):
    h = int(sec / 3600)
    sec -= h * 3600
    min = int(sec / 60)
    sec -= min * 60
    sec = int(sec)
    string = str(h) + ":" + str(min) + ":" +str(sec)
    return string

def continue_trad(data, samplerate):
    controlPass = True
    i = 0
    while i < len(data):
        if not data[i].recognised:
            duration = len(data[i].audio) / samplerate
            if duration / 2 < min_duration:
                data[i].text = "audio tra " + time_trad(data[i].start / samplerate) + " e " + time_trad(data[i].stop / samplerate) + " non riconosciuto"
            else:
                data_tmp = data[i].copy()
                data[i].audio = data[i].audio[0:int(len(data[i].audio) / 2)]
                data[i].stop = data[i].start + len(data[i].audio)
                data_tmp.audio = data_tmp.audio[len(data[i].audio) - 2 * samplerate:]
                data_tmp.start = data[i].stop - 2 * samplerate
                data.insert(i + 1, data_tmp.copy())
                i += 1
                controlPass = False
        i += 1
    return controlPass 


def recognise(data,samplerate):
    tmp_name = name_file_audio + "_tmp.wav"
    for i in range(len(data)):
        if data[i].text == "":
            sf.write(tmp_name, data[i].audio, samplerate)
            wav = sr.AudioFile(tmp_name)
            r = sr.Recognizer()
            with wav as source:
                audio = r.record(source)
            try:
                s = r.recognize_google(audio, language="it-IT")
                data[i].text += s
                data[i].recognised = True
            except sr.UnknownValueError:
                print("Google Speech Recognition could not understand audio")
                data[i].text += "testo non riconosciuto tra " + (time_trad(int(name_file_audio) * int(duration))) + " e " + (time_trad((1 + int(name_file_audio)) * int(duration)))


def write_file(data):
    f = open(name_file_audio + ".txt", "w")
    for i in range(len(data)):
        if data[i].recognised:
            f.write(data[i].text)
            f.write("\n")
    f = open(name_file_audio + "_failed.txt", "w")
    for i in range(len(data)):
        if not data[i].recognised:
            f.write(data[i].text)
            f.write("\n")
    

continue_rec = True 
data, samplerate = fill_audio(name_file_audio + ".wav")
recognise(data, samplerate)
write_file(data)
os.remove(name_file_audio + "_tmp.wav")
os.remove(name_file_audio + ".wav")



