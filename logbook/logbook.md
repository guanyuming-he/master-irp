# Logbook

## Meeting (30 May 2025)
*Present: Guanyuming He (GH, student), Sean O'Grady (SO, main supervisor)*

### Key points discussed:
Sean briefed me on the project background, the initiative to develop a software
system, why the state-of-the-art systems are not enough, and his general
requirements for me.

### Feedback received:
None. It is the first meeting.

### Work plan before next meeting:
I will on a literature review of the background and current systems, and give
an initial and rough plan of it.

## Meeting (06 June 2025)
*Present: Guanyuming He (GH, student), Sean O'Grady (SO, main supervisor),
Rhodri Nelson (RN, second supervisor)*

### Key points discussed:
I briefed them my literature review and very rough plan.

### Feedback received:
- They believed I could use less works for the background review.
- Sean thought my plan captures his requirements well.
- Rhodri thought my design is plausible, but it's important to get a prototype
  fist.

### Work plan before next meeting:
I will polish my plan before the deadline, and try to work on a prototype as
much as I can.

## Meeting (13 June 2025)
*Present: Guanyuming He (GH, student), Sean O'Grady (SO, main supervisor)*

### Key points discussed:
I presented my finished project plan and a small LLM prototype that can for now
turn text into search engine prompts.

### Feedback received:
- He thought my plan was good.
- He thought the prompts were close to the text given, but he thought they were
  too academic. Need to direct the search engines towards real examples.

### Work plan before next meeting:
I will improve that part and also continue working on a custom search
component.

## Meeting (20 June 2025)
*Present: Guanyuming He (GH, student), Sean O'Grady (SO, main supervisor)*

### Key points discussed:
Sean let me know he could not attend the meeting, so I sent him a text message
containing my progress that week: building the scraping and parsing part of my
search engine.

I also sent a copy of it to Rhodri.

### Feedback received:
N/A

### Work plan before next meeting:
I will finish the search engine next week.

## Meeting (27 June 2025)
*Present: Guanyuming He (GH, student), Sean O'Grady (SO, main supervisor)*

### Key points discussed:
I basically finishes the main functions of the search engine, and presented it
the Sean. For queries regarding objects, e.g. Donald Trump, tariffs, it worked
well. However, it didn't work well when given questions or indirect sentences.
The LLM also didin't work well to transfer them into good seach engine prompts.

Another problem is that the database was highly imbalanced towards business
insider.

### Feedback received:
Sean said he would give me a few topics to feed into my tool, and asked me to
sent him the results, about which he would give feedbacks on. He also said he
would send them to Rhodri to request his comments on how to tune the LLM better
for that.

### Work plan before next meeting:
I will wait for his feed back on that, while try to balance the database of my
search engine.

## Meeting (7 July 2025)
*Present: Guanyuming He (GH, student), Rhodri Nelson (RN, second supervisor)*

### Key points discussed:
Sean wanted Rhodri to meet me so that we can discuss technical points. Because
of his time slots, the meeting happened on next Monday, not the previous Friday
when it was supposed to happen.

I showed him how I improved my search engine and LLM tuning

### Feedback received:
He commented out that I should start working on using LLMs to synthesize my
retrieved data from my search engine. Even if the results are not quite so
satisfactory, I could still use a LLM to rerank the search results.
Another reason for my building my own search engine is the commercial search
engine's query quotas; it's already quite limiting for only searching text, not
to mention with AI it can only be more expensive. I told him this problem, and
he recommend another tool, Open WebUI to me, which I will investigate.

### Work plan before next meeting:
I will work on the two points Rhodri mentioned.

## Meeting (11 July 2025)
*Present: Guanyuming He (GH, student), Sean O'Grady (SO, main supervisor)*

### Key points discussed:
Sean had another meeting at the same time, so he asked me to send him my
progress.

I showed him that I built a pipeline that at that time could automatically read
input from config, generate search engine prompts with LLM, search, and
summarize with LLM.

### Feedback received:
Sean planned to give me feedback together in the next meeting.

### Work plan before next meeting:
I made a GUI to help configuring the pipeline while waiting for feedback.


## Meeting (18 July 2025)
*Present: Guanyuming He (GH, student), Sean O'Grady (SO, main supervisor)*

### Key points discussed:
I showed Sean the pipeline and the GUI configuration tool. I also showed him
the Open-WebUI Rhodri recommended to me with Google's Programmable Search
Engine: its limitations and why I don't want to use it.

### Feedback received:
Sean commented that I made good progress, but the final result is still at some
distance away from desired. Sean said he will send an email to both Rhodri and
me, discussing the situation, seeking his professional knowledge on how to make
LLMs better suited for our purpose.

### Work plan before next meeting:
I will tune the LLMs better, especially after I have Rhodri's reply to Sean's
email.


## Meeting (25 July 2025)
Delayed to a later date when Rhodri would be available.

## Meeting (1 August 2025)
*Present: Guanyuming He (GH, student), Sean O'Grady (SO, main supervisor)*

### Key points discussed:
I showed Sean my progress:
1. Fixed various bugs in the pipeline; made email delivery and schedulling
   working in all situations (I hope)
2. I integrated Google search into my pipeline as well.
3. I have been working to write my methods in my thesis.

### Feedback received:
Sean was pleased that the bugs were fixed that the whole automatic pipeline is
working. Sean said it was good that I also work on my writing in the meantime.
Sean said he will arrange a meeting with Rhodri on how to make the LLM more
stable.

### Work plan before next meeting:
I will continue polishing my thesis and then wait for the meeting with Rhodri
to tune the LLM to become more stable.


## Meeting (5 August 2025)
*Present: Guanyuming He (GH, student), Sean O'Grady (SO, main supervisor),
Rhodri Nelson (RN, second supervisor)*

### Key points discussed:
Sean and me presented the remaining problem: how to tune LLM better for our
tasks to Rhodri in the hope of getting his expert comments. I showed them how I
used the tool recommend by Rhodri, Open-WebUI, to tune the LLM, but to no
avail.

### Feedback received:
Rhodri commented that, since tuning the system prompt failed despite my
efforts, it probably follows that the current one-shot LLM application won't
work greatly. He suggested me to make it multi-stage instead of one-shot.

### Work plan before next meeting:
I will apply Rhodri's suggestion to my system.


