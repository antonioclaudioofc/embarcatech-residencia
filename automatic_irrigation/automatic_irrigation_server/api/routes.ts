import { FastifyInstance } from "fastify";
import { realtime } from "./firebase";

export async function routes(app: FastifyInstance) {
  app.post("/irrigation", async (request, reply) => {
    const body = request.body as {
      time: string;
      days: string[];
      specificDates: string[];
      duration: number;
    };

    const ref = realtime.ref("irrigation").push();
    await ref.set({
      ...body,
      createdAt: Date.now(),
    });

    return reply.status(201).send({ ok: true, id: ref.key });
  });

  app.get("/irrigation", async (_, reply) => {
    const snapshot = await realtime.ref("irrigation").once("value");
    const data = snapshot.val() || {};
    return reply.send(data);
  });
}
